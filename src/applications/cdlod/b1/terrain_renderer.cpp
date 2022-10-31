
#include "terrain_renderer.h"
#include "lod_selection.h"
#include "applications/camera/camera.h"
#include "node.h"
#include "quadtree.h"
#include "heightmap.h"
#include "renderer/uniform.h"
#include "scene/scene.h"
#include "base/logbook.h"
#include "omath/aabb.h"
#include "omath/mat4.h"
#include "renderer/program.h"
#include "renderer/uniform.h"
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>

using namespace orf_n;

namespace terrain {

terrain_renderer::terrain_renderer() : renderable( "TerrainLOD" ) {
	// Prepare and check settings
	if( !omath::is_power_of_2( settings::LEAF_NODE_SIZE ) ||
			settings::LEAF_NODE_SIZE < 2 || settings::LEAF_NODE_SIZE > 1024 ) {
		std::string s{ "Settings LEAF_NODE_SIZE must be power of 2 and between 2 and 1024." };
		logbook::log_msg( logbook::TERRAIN, logbook::WARNING, s );
	}
	if( !omath::is_power_of_2( settings::RENDER_GRID_RESULUTION_MULT ) || settings::RENDER_GRID_RESULUTION_MULT < 1 ) {
		std::string s{ "Settings RENDER_GRID_RESULUTION_MULT must be power of 2 and between 1 and LEAF_NODE_SIZE." };
		logbook::log_msg( logbook::TERRAIN, logbook::ERROR, s );
		throw std::runtime_error( s );
	}
	if( settings::NUMBER_OF_LOD_LEVELS < 2 || settings::NUMBER_OF_LOD_LEVELS > 15 ) {
		std::string s{ "Settings NUMBER_OF_LOD_LEVELS must be between 1 and 15." };
		logbook::log_msg( logbook::TERRAIN, logbook::WARNING, s );
	}
	if( settings::LOD_LEVEL_DISTANCE_RATIO < 1.5f || settings::LOD_LEVEL_DISTANCE_RATIO > 16.0f ) {
		std::string s{ "Settings LOD_LEVEL_DISTANCE_RATIO must be between 1.5f and 16.0f." };
		logbook::log_msg( logbook::TERRAIN, logbook::WARNING, s );
	}
	if( !omath::is_power_of_2( settings::GRIDMESH_DIMENSION ) ||
			settings::GRIDMESH_DIMENSION < 8 || settings::GRIDMESH_DIMENSION > 1024 ) {
		std::string s{ "Gridmesh dimension must be power of 2 and > 8 and < 1024." };
		logbook::log_msg( logbook::TERRAIN, logbook::WARNING, s );
	}

	// Prepare gridmesh for drawing and load terrain tiles
	m_gridmesh = std::make_unique<gridmesh>( settings::GRIDMESH_DIMENSION );
	// Load the heightmap and tile relative and world min/max coords for the bounding boxes
	// TODO: check size and if it fits quadtree, do a proper datastructure and asynchronous load.
	const std::vector<std::string> terrain_files {
		//"/home/kemde/eclipse-workspace/cdlod_backup/resources/textures/terrain/n30e090/tiles_16k/tile_1638_4",
		"resources/textures/terrain/n30e090/tiles_4k/tile_4096_4"
	};
	m_heightmap = std::make_unique<heightmap>( terrain_files[0] + ".png", heightmap::B16 );
	std::ifstream bbf( terrain_files[0] + ".bb", std::ios::in );
	if( !bbf.is_open() ) {
		std::ostringstream s;
		s << "Error opening bounding box file '" << terrain_files[0] << ".bb'. Tile will not be rendered correctly.";
		orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::WARNING, s.str() );
	}
	omath::dvec3 min, max;
	bbf >> min.x >> min.y >> min.z >> max.x >> max.y >> max.z;
	bbf.close();
	m_aabb = std::make_unique<omath::aabb>(
			omath::dvec3{ min.x, min.y * settings::HEIGHT_FACTOR, min.z },
			omath::dvec3{ max.x, max.y * settings::HEIGHT_FACTOR, max.z }
	);

	// Build quadtree with nodes and their bounding boxes.
	m_quadtree = std::make_unique<quadtree>( this );
	m_quadtree->create();

	// TODO class terrain_tile
	std::ostringstream s;
	s << "Terrain tile '" << terrain_files[0] << "'loaded.";
	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO, s.str() );
	s.str( std::string() );
	s << "\tBounding box of whole world: " << *m_aabb;
	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO, s.str() );

	// Create terrain shaders
	std::vector<std::shared_ptr<module>> modules;
	modules.push_back(
			std::make_shared<module>(GL_VERTEX_SHADER, "src/applications/cdlod/terrain.vert.glsl" )
	);
	modules.push_back(
			std::make_shared<module>( GL_FRAGMENT_SHADER,"src/applications/cdlod/terrain.frag.glsl" )
	);
	m_shaderTerrain = std::make_unique<program>( modules );
}

const heightmap *terrain_renderer::get_heightmap() const {
	return m_heightmap.get();
}

const terrain::quadtree *terrain_renderer::getQuadTree() const {
	return m_quadtree.get();
}

const omath::aabb *terrain_renderer::get_aabb() const {
	return m_aabb.get();
}

terrain_renderer::~terrain_renderer() {
	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO, "Terrain renderer deleted." );
}

void terrain_renderer::setup() {
	// Camera and selection object. Are connected because selection is based on view frustum and range.
	// TODO parametrize or calculate initial position, direction and view range.
	m_scene->get_camera()->set_position_and_target( m_aabb->m_max, m_aabb->m_min );
	m_scene->get_camera()->set_near_plane( 1.0f );
	m_scene->get_camera()->set_far_plane( m_aabb->get_diagonal_size() );
	m_scene->get_camera()->calculate_fov();

	// @todo: no sorting for now. Should be sorted by tileIndex, distanceToCamera and lodLevel
	// to avoid too many heightmap switches and shader uniform settings.
	// Lod selection ranges depend on camera near/far plane distances
	m_selection = new lod_selection{ m_scene->get_camera(), false /*don't sort*/ };

	m_draw_aabb.setup();

	// Set global shader uniforms valid for all tiles
	m_shaderTerrain->use();
	// Set default global shader uniforms belonging to this quadtree/heightmap
	const float w = (float)m_heightmap->get_extent().x;
	const float h = (float)m_heightmap->get_extent().y;
	// Used to clamp edges to correct terrain size (only max-es needs clamping, min-s are clamped implicitly)
	m_tileToTexture = omath::vec2{ ( w - 1.0f ) / w, ( h - 1.0f ) / h };
	m_heightMapInfo = omath::vec4{ w, h, 1.0f / w, 1.0f / h };
	const GLuint p = m_shaderTerrain->get_program();
	set_uniform( p, "g_tileToTexture", m_tileToTexture );
	set_uniform( p, "g_heightmapTextureInfo", m_heightMapInfo );
	set_uniform( p, "u_height_factor", settings::HEIGHT_FACTOR );
	// Set dimensions of the gridmesh used for rendering an individual node
	set_uniform( p, "g_gridDim", omath::vec3{
		static_cast<float>(settings::GRIDMESH_DIMENSION),
		static_cast<float>(settings::GRIDMESH_DIMENSION) * 0.5f,
		2.0f / static_cast<float>(settings::GRIDMESH_DIMENSION)
	} );
	// @todo: Global lighting. In the long run, this will be done elsewhere ...
	const omath::vec4 lightColorAmbient{ 0.35f, 0.35f, 0.35f, 1.0f };
	const omath::vec4 lightColorDiffuse{ 0.65f, 0.65f, 0.65f, 1.0f };
	const omath::vec4 fogColor{ 0.0f, 0.5f, 0.5f, 1.0f };
	const omath::vec4 colorMult{ 1.0f, 1.0f, 1.0f, 1.0f };
	set_uniform( p, "g_lightColorDiffuse", lightColorDiffuse );
	set_uniform( p, "g_lightColorAmbient", lightColorAmbient );
	set_uniform( p, "g_colorMult", colorMult );
	set_uniform( p, "g_diffuseLightDir", -m_diffuseLightPos );
}

void terrain_renderer::render(const double deltatime) {
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	const camera *const cam{ m_scene->get_camera() };
	// Perform selection TODO parametrize sorting and concatenate lod selection.
	// Reset selection, add nodes, sort selection, lod level and nearest to farest.
	if( !m_single_step || (m_single_step && !m_stepped) ) {
		m_selection->reset();
		m_quadtree->lodSelect( m_selection );
		m_selection->set_distances_and_sort();
		if( m_single_step && m_print_selection )
			m_selection->print_selection();
		if( !m_stepped ) {
			logbook::log_msg(logbook::TERRAIN, logbook::INFO, "Step");
			m_stepped=true;
		}
	}

	// Debug: draw bounding boxes
	bool refreshUniforms{ refreshUI() };
	if( m_showTileBoxes || m_showLowestLevelBoxes || m_showSelectedBoxes )
		debugDrawing();

	// Bind meshes, shader, reset stats, prepare and set matrices and cam pos
	if( !m_drawSelection )
		return;
	m_gridmesh->bind();
	m_renderStats.reset();
	m_shaderTerrain->use();
	const GLuint p = m_shaderTerrain->get_program();
	if( refreshUniforms )
		set_uniform( p, "g_diffuseLightDir", -m_diffuseLightPos );
	setViewProjectionMatrix( cam->get_view_perspective_matrix() );
	set_uniform( p, "debugColor", color::white );
	set_uniform( p, "u_camera_position", omath::vec3(cam->get_position()) );
	// Draw tile by tile
	GLint drawMode{ cam->get_wireframe_mode() ? GL_LINES : GL_TRIANGLES };
	// TODO These are now world coords. Set tile world coords.
	set_uniform( p, "g_tileMax", omath::vec2{ m_aabb->m_max.x, m_aabb->m_max.z } );
	set_uniform( p, "g_tileScale", omath::vec3{ m_aabb->m_max - m_aabb->m_min } );
	set_uniform( p, "g_tileOffset", omath::vec3{ m_aabb->m_min } );

	omath::uvec2 renderStats{ 0, 0 };
	m_heightmap->bind();
	// Submeshes are evenly spaced in index buffer. Else calc offsets individually.
	const unsigned int halfD{ m_gridmesh->getEndIndexTL() };
	// Iterate through the lod selection's lod levels
	for( unsigned int level = m_selection->m_min_selected_lod_level; level <= m_selection->m_max_selected_lod_level; ++level ) {
		const unsigned int filterLODLevel = level;
		// TODO: gridmesh is originally picked here. Why ?
		unsigned int prevMorphConstLevelSet = UINT_MAX;
		for( unsigned int i=0; i < m_selection->m_selection_count; ++i ) {
			const lod_selection::selected_node &n = m_selection->m_selected_nodes[i];
			// Only draw tiles of the currently bound heightmap; filter out nodes if not of the current level
			if( filterLODLevel != n.lod_level )
				continue;
			// Set LOD level specific consts if they have changed from last lod level
			if( prevMorphConstLevelSet == UINT_MAX || prevMorphConstLevelSet != n.lod_level ) {
				prevMorphConstLevelSet = n.lod_level;
				orf_n::set_uniform(
						p, "g_morphConsts", m_selection->get_morph_consts( prevMorphConstLevelSet )
				);
			}
			bool drawFull{ n.has_tl && n.has_tr && n.has_bl && n.has_br };
			const omath::aabb bb = n.p_node->get_world_aabb();
			// .w holds the current lod level
			omath::vec4 nodeScale{ bb.get_size().x, 0.0f, bb.get_size().z, float(n.lod_level) };
			omath::vec3 nodeOffset{ bb.m_min.x, (bb.m_min.y+bb.m_max.y) * 0.5f, bb.m_min.z };
			orf_n::set_uniform( p, "g_nodeScale", nodeScale );
			orf_n::set_uniform( p, "g_nodeOffset", nodeOffset );
			const int numIndices{ m_gridmesh->get_number_indices() };
			if( drawFull ) {
				glDrawElements( drawMode, numIndices, GL_UNSIGNED_INT, (const void *)0 );
				++renderStats.x;
				renderStats.y += numIndices / 3;
			} else {
				// can be optimized by combining calls
				if( n.has_tl ) {
					glDrawElements( drawMode, halfD, GL_UNSIGNED_INT, (const void *)0 );
					++renderStats.x;
					renderStats.y += halfD / 3;
				}
				if( n.has_tr ) {
					glDrawElements(
							drawMode, halfD, GL_UNSIGNED_INT,(const void *)( m_gridmesh->getEndIndexTL() * sizeof( GL_UNSIGNED_INT ) )
					);
					++renderStats.x;
					renderStats.y += halfD / 3;
				}
				if( n.has_bl ) {
					glDrawElements(
							drawMode, halfD, GL_UNSIGNED_INT,(const void *)( m_gridmesh->getEndIndexTR() * sizeof( GL_UNSIGNED_INT ) )
					);
					++renderStats.x;
					renderStats.y += halfD / 3;
				}
				if( n.has_br ) {
					glDrawElements(
							drawMode, halfD, GL_UNSIGNED_INT,(const void *)( m_gridmesh->getEndIndexBL() * sizeof( GL_UNSIGNED_INT ) )
					);
					++renderStats.x;
					renderStats.y += halfD / 3;
				}
			}
		}
	}
	m_renderStats.totalRenderedNodes += renderStats.x;
	m_renderStats.totalRenderedTriangles += renderStats.y;
}

void terrain_renderer::cleanup() {
	delete m_selection;
	m_draw_aabb.cleanup();
}

// ******** Debug stuff
void terrain_renderer::debugDrawing() {
	// To keep the below less verbose
	const program *p{ m_draw_aabb.getProgramPtr() };
	p->use();
	set_uniform( p->get_program(), "projViewMatrix", m_scene->get_camera()->get_view_perspective_matrix() );
	if( m_showTileBoxes )
		m_draw_aabb.draw( *( m_aabb ), color::white );
	if( m_showLowestLevelBoxes )
		debugDrawLowestLevelBoxes();
	if( m_showSelectedBoxes ) {
		for( unsigned int i=0; i < m_selection->m_selection_count; ++i ) {
			const lod_selection::selected_node &n = m_selection->m_selected_nodes[i];
			bool drawFull = n.has_tl && n.has_tr && n.has_bl && n.has_br;
			if( drawFull )
				m_draw_aabb.draw(
						n.p_node->get_world_aabb().expand( -0.003f ),color::rainbow[n.p_node->get_level()]
				);
			else {
				if( n.has_tl )
					m_draw_aabb.draw(
							n.p_node->get_tl()->get_world_aabb().expand( -0.002f ),color::rainbow[n.p_node->get_tl()->get_level()]
					);
				if( n.has_tr )
					m_draw_aabb.draw(
							n.p_node->get_tr()->get_world_aabb().expand( -0.002f ),color::rainbow[n.p_node->get_tr()->get_level()]
					);
				if( n.has_bl )
					m_draw_aabb.draw(
							n.p_node->get_bl()->get_world_aabb().expand( -0.002f ),color::rainbow[n.p_node->get_bl()->get_level()]
					);
				if( n.has_br )
					m_draw_aabb.draw(
							n.p_node->get_br()->get_world_aabb().expand( -0.002f ),color::rainbow[n.p_node->get_br()->get_level()]
					);
			}
		}
	}
}

// Draw all bounding boxes of heightmap
void terrain_renderer::debugDrawLowestLevelBoxes() const {
	const node *const nodes{ m_quadtree->getNodes() };
	for( unsigned int i=0; i < m_quadtree->getNodeCount(); ++i )
		if( nodes[i].get_level() == settings::NUMBER_OF_LOD_LEVELS - 1 ) {
			const omath::aabb box = nodes[i].get_world_aabb();
			if( m_scene->get_camera()->get_view_frustum().is_box_in_frustum( box ) != omath::OUTSIDE )
				m_draw_aabb.draw( box, color::cornflowerBlue );
		}
}

bool terrain_renderer::refreshUI() {
	bool retVal{ false };
	// UI stuff
	ImGui::Begin( "Basic LOD params" );
	ImGui::Text( "Show bounding boxes:" );
	ImGui::Checkbox( "  of tiles map", &m_showTileBoxes );
	ImGui::Checkbox( "  in viewfrustum", &m_showLowestLevelBoxes );
	ImGui::Checkbox( "  LOD selected", &m_showSelectedBoxes );
	ImGui::Checkbox( "Show terrain", &m_drawSelection );
	ImGui::Checkbox( "Single step", &m_single_step );
	if(m_single_step) {
		ImGui::SameLine();
		if( ImGui::Button( "Perform Step" ) )
			m_stepped=false;
		ImGui::Checkbox( "Print selection", &m_print_selection );
	} else
		// Reset this in case user forgets.
		m_print_selection = false;
	ImGui::Separator();
	ImGui::Text( "Render stats" );
	ImGui::Text( "# selected nodes %d", m_selection->m_selection_count );
	ImGui::Text( "# rendered nodes %d", m_renderStats.totalRenderedNodes );
	ImGui::Text( "# rendered triangles %d", m_renderStats.totalRenderedTriangles );
	ImGui::Text( "min selected LOD level %d", m_selection->m_min_selected_lod_level );
	ImGui::Text( "max selected LOD level %d", m_selection->m_max_selected_lod_level );
	ImGui::Separator();
	float nearPlane{ m_scene->get_camera()->get_near_plane() };
	float farPlane{ m_scene->get_camera()->get_far_plane() };
	const omath::vec3 oldDiffuseLightPos{ m_diffuseLightPos };
	ImGui::Text( "Camera Control" );
	ImGui::SliderFloat( "Near plane", &nearPlane, 1.0f, 100.0f );
	ImGui::SliderFloat( "Far plane", &farPlane, 200.0f, 10000.0f );
	ImGui::SliderFloat( "Light Position x", &m_diffuseLightPos.x, -1.0f, 1.0f );
	ImGui::End();
	// Recalc camera fov and lod ranges on change
	if( nearPlane != m_scene->get_camera()->get_near_plane() ) {
		m_scene->get_camera()->set_near_plane( nearPlane );
		m_selection->calculate_ranges();
	}
	if( farPlane != m_scene->get_camera()->get_far_plane()  ) {
		m_scene->get_camera()->set_far_plane( farPlane );
		m_selection->calculate_ranges();
	}
	if( oldDiffuseLightPos != m_diffuseLightPos )
		retVal = true;

	return retVal;
}

}
