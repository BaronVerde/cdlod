
#pragma once

#include "settings.h"
#include "applications/camera/camera.h"
#include "omath/vec4.h"
#include <climits>

namespace terrain {

class node;
class quadtree;

class lod_selection {
public:
	typedef struct selected_node {
		node *p_node{ nullptr };
		// First bit is used to mark too short visibility ranges.
		unsigned int lod_level = UINT_MAX;
		bool has_tl{ false };
		bool has_tr{ false };
		bool has_bl{ false };
		bool has_br{ false };
		double min_distance_to_camera{ 0.0 };	// for sorting by distance
		selected_node() {};
		selected_node( node *n, unsigned int lvl, bool tl, bool tr, bool bl, bool br ) :
			p_node{n}, lod_level{lvl}, has_tl{tl}, has_tr{tr}, has_bl{bl}, has_br{br} {}
		bool is_vis_dist_too_small() const;
	} selected_node;

	lod_selection( const orf_n::camera *cam, bool sortByDistance = false );
	virtual ~lod_selection();
	// Called when camera near or far plane changed to recalc visibility and morph ranges.
	void calculate_ranges();
	void set_distances_and_sort();
	// Here all parameters are set. TODO parametrize sorting and stop level.
	void reset();
	void print_selection() const;
	const omath::vec4 get_morph_consts( const unsigned int lodLevel ) const;

	const orf_n::camera *m_camera{ nullptr };
	selected_node m_selected_nodes[settings::MAX_NUMBER_SELECTED_NODES];

	double m_visibility_ranges[settings::NUMBER_OF_LOD_LEVELS];
	bool m_sort_by_distance = false;
	bool m_vis_dist_too_small = false;
	double m_morph_start[settings::NUMBER_OF_LOD_LEVELS];
	double m_morph_end[settings::NUMBER_OF_LOD_LEVELS];
	// Stop at this level when selecting nodes. Can accelarate the process for only far away terrain.
	unsigned int m_stop_at_level = settings::NUMBER_OF_LOD_LEVELS-1;
	unsigned int m_selection_count = 0;
	unsigned int m_max_selected_lod_level = 0;
	unsigned int m_min_selected_lod_level = settings::NUMBER_OF_LOD_LEVELS-1;

	void debug_output_morph_levels() const;

};

}
