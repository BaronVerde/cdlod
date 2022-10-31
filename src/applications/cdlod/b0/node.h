
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
	 * LOD level equals 'settings::NUM_LOD_LEVELS - 1 - node::get_level()' */
	unsigned int get_level() const;
	unsigned int get_size() const;
	// Get the aabb in world coordinates for selection.
	const omath::aabb &get_aabb() const;
	void get_world_aabb(omath::daabb &box) const;
	// Set a nodes height when heightmap data becomes available
	void set_min_max_height();
	//const omath::vec2 &get_min_max_height() const;
	// worldPositionCellsize: .x = lower left latitude, .y = longitude, .z = cellsize
    void create(
    		const unsigned int x, const unsigned int z, const unsigned int size, const unsigned int level,
    		const terrain_renderer *const lod, node *all_nodes, unsigned int &last_index );
    omath::t_intersect lod_select( lod_selection *selection, bool parent_completely_in_frustum = false );
    const node *get_tr() const;
    const node *get_tl() const;
    const node *get_br() const;
    const node *get_bl() const;

private:
    unsigned int m_x;
	unsigned int m_z;
	// First bit is used to mark a leaf node.
	unsigned int m_level;

    node *m_tl{ nullptr };
    node *m_tr{ nullptr };
    node *m_bl{ nullptr };
    node *m_br{ nullptr };

    omath::aabb m_aabb;

};

}
