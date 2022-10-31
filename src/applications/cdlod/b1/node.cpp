
#include <applications/cdlod/terrain_renderer.h>
#include "lod_selection.h"
#include "heightmap.h"
#include "node.h"
#include "quadtree.h"
#include "base/logbook.h"
#include "omath/aabb.h"
#include "omath/view_frustum.h"
#include <sstream>

using namespace orf_n;

namespace terrain {

node::node() {}

void node::create(
		const unsigned int x, const unsigned int z, const unsigned int size, const unsigned int level,
		const terrain_renderer *const lod, node *allNodes, unsigned int &lastIndex ) {
	m_x = x;
	m_z = z;
	m_level = level;
	m_size = size;
	const heightmap *h_map{ lod->get_heightmap() };
	// Find min/max heights at this patch of terrain
	const unsigned int limitX = std::min( h_map->get_extent().x, x + size + 1 );
	// limit z = y-axis of heightmap
	const unsigned int limitZ = std::min( h_map->get_extent().y, z + size + 1 );
	m_min_max_height = h_map->get_min_max_height_area( x, z, limitX - x, limitZ - z );
	// TODO Get bounding box in world coords.
	const omath::vec3 min{ lod->get_aabb()->m_min.x + (float)m_x, m_min_max_height.x, lod->get_aabb()->m_min.z + (float)m_z };
	const omath::vec3 max{
		lod->get_aabb()->m_min.x + float(m_x + m_size), m_min_max_height.y, lod->get_aabb()->m_min.z + float(m_z + m_size)
	};
	m_boundingBox = omath::aabb{ min, max };
	// Highest level reached already ?
	if( size == settings::LEAF_NODE_SIZE ) {
		if( level != settings::NUMBER_OF_LOD_LEVELS -1 ) {
			std::string s{ "Lowest lod level unequals number lod levels during quad tree node creation." };
			logbook::log_msg( logbook::TERRAIN, logbook::ERROR, s );
			throw std::runtime_error( s );
		}
		// Mark leaf node!
	    m_level |= 0x80000000;
	} else {
		unsigned int subSize = size / 2;
		m_tl = &allNodes[lastIndex++];
		m_tl->create( x, z, subSize, level+1, lod, allNodes, lastIndex );
		if( ( x + subSize ) < h_map->get_extent().x ) {
	         m_tr = &allNodes[lastIndex++];
	         m_tr->create( x + subSize, z, subSize, level+1, lod, allNodes, lastIndex );
		}
		if( (z + subSize) < h_map->get_extent().y ) {
			m_bl = &allNodes[lastIndex++];
			m_bl->create( x, z + subSize, subSize, level+1, lod, allNodes, lastIndex );
		}
		if( ( ( x + subSize ) < h_map->get_extent().x ) && ( ( z + subSize ) < h_map->get_extent().y ) ) {
			m_br = &allNodes[lastIndex++];
			m_br->create( x + subSize, z + subSize, subSize, level+1, lod, allNodes, lastIndex );
		}
	}
}

node::~node() {}

const omath::vec2 &node::get_min_max_height() const {
	return m_min_max_height;
}

unsigned int node::get_level() const {
	return m_level & 0x7FFFFFFF;
}

const omath::aabb &node::get_world_aabb() const {
	return m_boundingBox;
}

const node *node::get_tr() const {
	return m_tr;
}

const node *node::get_tl() const {
	return m_tl;
}

const node *node::get_br() const {
	return m_br;
}

const node *node::get_bl() const {
	return m_bl;
}

bool node::is_leaf() const {
	return (m_level & 0x80000000) != 0;
}

omath::intersect_t node::lod_select( lod_selection *lodSelection, bool parentCompletelyInFrustum ) {
	// Shortcut
	const camera *cam{ lodSelection->m_camera };
	// Test early outs
	omath::view_frustum f = cam->get_view_frustum();
	omath::intersect_t frustumIntersection = parentCompletelyInFrustum ?
			omath::INSIDE : cam->get_view_frustum().is_box_in_frustum( m_boundingBox );
	if( omath::OUTSIDE == frustumIntersection )
		return omath::OUTSIDE;
	float distanceLimit = lodSelection->m_visibility_ranges[get_level()];
	if( !m_boundingBox.intersect_sphere_sq( cam->get_position(), distanceLimit * distanceLimit ) )
		return omath::OUT_OF_RANGE;

	omath::intersect_t subTLSelRes = omath::UNDEFINED;
	omath::intersect_t subTRSelRes = omath::UNDEFINED;
	omath::intersect_t subBLSelRes = omath::UNDEFINED;
	omath::intersect_t subBRSelRes = omath::UNDEFINED;
	// Stop at one below number of lod levels
	if( get_level() != lodSelection->m_stop_at_level ) {
		float nextDistanceLimit = lodSelection->m_visibility_ranges[get_level()+1];
		if( m_boundingBox.intersect_sphere_sq( cam->get_position(), nextDistanceLimit * nextDistanceLimit ) ) {
			bool weAreCompletelyInFrustum = frustumIntersection == omath::INSIDE;
			if( m_tl != nullptr )
				subTLSelRes = m_tl->lod_select( lodSelection, weAreCompletelyInFrustum );
			if( m_tr != nullptr )
				subTRSelRes = m_tr->lod_select( lodSelection, weAreCompletelyInFrustum );
			if( m_bl != nullptr )
				subBLSelRes = m_bl->lod_select( lodSelection, weAreCompletelyInFrustum );
			if( m_br != nullptr )
				subBRSelRes = m_br->lod_select( lodSelection, weAreCompletelyInFrustum );
		}
	}

	// We don't want to select sub nodes that are invisible (out of frustum) or are selected,
	// we DO want to select if they are out of range, since we are not.
	bool removeSubTL = (subTLSelRes == omath::OUTSIDE) || (subTLSelRes == omath::SELECTED);
	bool removeSubTR = (subTRSelRes == omath::OUTSIDE) || (subTRSelRes == omath::SELECTED);
	bool removeSubBL = (subBLSelRes == omath::OUTSIDE) || (subBLSelRes == omath::SELECTED);
	bool removeSubBR = (subBRSelRes == omath::OUTSIDE) || (subBRSelRes == omath::SELECTED);

	if( lodSelection->m_selection_count >= settings::MAX_NUMBER_SELECTED_NODES ) {
		std::string s{ "LOD selected more nodes than the maximum selection count. Some nodes will not be drawn." };
		logbook::log_msg( logbook::TERRAIN, logbook::WARNING, s );
		return omath::OUTSIDE;
	}
	// Add node to selection
	lod_selection::selected_node *snode = &lodSelection->m_selected_nodes[lodSelection->m_selection_count];
	if( !( removeSubTL && removeSubTR && removeSubBL && removeSubBR ) &&
		 ( lodSelection->m_selection_count < settings::MAX_NUMBER_SELECTED_NODES ) ) {
		unsigned int lodLevel = lodSelection->m_stop_at_level - get_level();
		*snode = lod_selection::selected_node(
				this, lodLevel, !removeSubTL, !removeSubTR, !removeSubBL, !removeSubBR
		);
		lodSelection->m_min_selected_lod_level = std::min( lodSelection->m_min_selected_lod_level, snode->lod_level );
		lodSelection->m_max_selected_lod_level = std::max( lodSelection->m_max_selected_lod_level, snode->lod_level );
		// TODO Check if we get problems with lod distnce ranges.
		// F.Strugar says: This should be calculated somehow better, but brute force will work for now.
		if( !lodSelection->m_vis_dist_too_small && (get_level() != 0) ) {
			const float maxDistFromCam = std::sqrt( m_boundingBox.max_distance_from_point_sq( cam->get_position() ) );
			const float morphStartRange = lodSelection->m_morph_start[lodSelection->m_stop_at_level - get_level()+1];
			if( maxDistFromCam > morphStartRange ) {
				lodSelection->m_vis_dist_too_small = true;
				// TODO mark offending box for drawing.
			}
		}
		// Set tile index, min distance and min/max levels for sorting
		if( lodSelection->m_sort_by_distance )
			snode->min_distance_to_camera = std::sqrt(
					snode->p_node->get_world_aabb().min_distance_from_point_sq( cam->get_position() )
			);
		lodSelection->m_selection_count++;
		return omath::SELECTED;
	}
	// if any of child nodes are selected, then return selected -
	// otherwise all of them are out of frustum, so we're out of frustum too
	if( (subTLSelRes == omath::SELECTED) || (subTRSelRes == omath::SELECTED) ||
		(subBLSelRes == omath::SELECTED) || (subBRSelRes == omath::SELECTED) )
		return omath::SELECTED;
	else
		return omath::OUTSIDE;
}

}

/*
 * 	    // Find heights for 4 corner points (used for approx ray casting)
	    // (reuse otherwise empty pointers used for sub nodes)
	    float * pTLZ = (float *)&subTL;
	    float * pTRZ = (float *)&subTR;
	    float * pBLZ = (float *)&subBL;
	    float * pBRZ = (float *)&subBR;
	    int limitX = ::min( rasterSizeX - 1, x + size );
	    int limitY = ::min( rasterSizeY - 1, y + size );
	    *pTLZ = createDesc.MapDims.MinZ + createDesc.pheightmap->GetHeightAt( x, y ) * createDesc.MapDims.SizeZ / 65535.0f;
	    *pTRZ = createDesc.MapDims.MinZ + createDesc.pheightmap->GetHeightAt( limitX, y ) * createDesc.MapDims.SizeZ / 65535.0f;
	    *pBLZ = createDesc.MapDims.MinZ + createDesc.pheightmap->GetHeightAt( x, limitY ) * createDesc.MapDims.SizeZ / 65535.0f;
	    *pBRZ = createDesc.MapDims.MinZ + createDesc.pheightmap->GetHeightAt( limitX, limitY ) * createDesc.MapDims.SizeZ / 65535.0f;
 */
