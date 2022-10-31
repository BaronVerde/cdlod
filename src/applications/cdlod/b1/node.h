
#pragma once

#include "omath/aabb.h"
#include "omath/vec2.h"
#include "omath/view_frustum.h"
#include <memory>

namespace terrain {

class heightmap;
class lod_selection;
class terrain_renderer;

class node {
public:
	node();
	virtual ~node();
	bool is_leaf() const;
	/* Level 0 is a root node, and level 'lod_level-1' is a leaf node. So the actual
	 * LOD level equals 'lod_level_count - 1 - node::get_level()' */
	unsigned int get_level() const;
	const omath::aabb &get_world_aabb() const;
	const omath::vec2 &get_min_max_height() const;
	// worldPositionCellsize: .x = lower left latitude, .y = longitude, .z = cellsize
    void create(
    		const unsigned int x, const unsigned int z, const unsigned int size, const unsigned int level,
    		const terrain_renderer *const lod, node *all_nodes, unsigned int &last_index );
    omath::intersect_t lod_select( lod_selection *selection, bool parent_completely_in_frustum = false );
    const node *get_tr() const;
    const node *get_tl() const;
    const node *get_br() const;
    const node *get_bl() const;

private:
    unsigned int m_x;
    unsigned int m_z;
    unsigned int m_size;
	unsigned int m_level;
	omath::vec2 m_min_max_height;

    node *m_tl{ nullptr };
    node *m_tr{ nullptr };
    node *m_bl{ nullptr };
    node *m_br{ nullptr };

    omath::aabb m_boundingBox;

};

}
