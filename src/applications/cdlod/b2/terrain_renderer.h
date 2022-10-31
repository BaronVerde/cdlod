
#pragma once

#include "gridmesh.h"
#include "quadtree.h"
#include "settings.h"
#include "scene/renderable.h"
#include "renderer/color.h"
#include "renderer/program.h"
#include <memory>
#include "aabb_drawing.h"

namespace terrain {

class lod_selection;
class heightmap;
class gridmesh;
class quadtree;

class terrain_renderer : public orf_n::renderable {
public:

	terrain_renderer();
	virtual ~terrain_renderer();
	terrain_renderer( const terrain_renderer &other ) = delete;
	terrain_renderer &operator=( const terrain_renderer &other ) = delete;
	terrain_renderer( terrain_renderer &&other ) = default;
	terrain_renderer &operator=( terrain_renderer &&other ) = default;

	const heightmap *get_heightmap() const;
	const quadtree *get_quadtree() const;

	virtual void setup() override final;
	virtual void render(const double deltatime) override final;
	virtual void cleanup() override final;

private:

	bool m_check_passed = true;
	bool check_settings();

	std::unique_ptr<heightmap> m_heightmap{nullptr};
	std::unique_ptr<quadtree> m_quadtree{nullptr};

	struct renderStats_t {
		int totalRenderedNodes{ 0 };
		int totalRenderedTriangles{ 0 };
		void reset() {
			totalRenderedTriangles = totalRenderedNodes = 0;
		}
	} m_renderStats;

	std::unique_ptr<gridmesh> m_gridmesh{ nullptr };
	std::unique_ptr<orf_n::program> m_shaderTerrain{ nullptr };
	terrain::lod_selection *m_selection{ nullptr };
	/* These figures are identical for all tiles of the same size. They hold the texture sizes
	 * and their ratio tile to texture. This implies that all tiles must have equal size
	 * and all textures equal resolution. Otherwise these values would have to be tile specific. */
	omath::vec2 m_tileToTexture;
	omath::vec4 m_heightMapInfo;
	// Lighting TODO, and it is the direction, not the position.
	omath::vec3 m_diffuseLightPos{ -1.0f, 1.0f, 0.0f };
	omath::mat4 m_modelMatrix{ 1.0f };
	// Returns true when shader uniforms need to be updated.
	bool refreshUI();

	// **** Debug stuff ****
	aabb_drawing &m_draw_aabb{ aabb_drawing::getInstance() };
	void debugDrawing();
	void debugDrawLowestLevelBoxes() const;
	bool m_showTileBoxes{ false };
	bool m_showLowestLevelBoxes{ false };
	bool m_showSelectedBoxes{ false };
	bool m_drawSelection{ true };
	bool m_single_step{false};
	bool m_stepped{true};
	bool m_print_selection{false};

};

}
