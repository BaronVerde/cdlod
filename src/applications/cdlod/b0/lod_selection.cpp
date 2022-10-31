
#include "lod_selection.h"
#include "node.h"
#include "quadtree.h"
#include "base/logbook.h"
#include "omath/common.h"	// lerp()
#include <sstream>
#include <iostream>

using namespace orf_n;

namespace terrain {

lod_selection::lod_selection( const camera *cam, bool sort ) :
		m_camera{ cam }, m_sort_by_distance{ sort } {
	calculate_ranges();
}

lod_selection::~lod_selection() {}

void lod_selection::calculate_ranges() {
	double total{ 0 };
	double current_detail_balance{ 1.0f };
	for( unsigned int i=0; i < settings::NUMBER_OF_LOD_LEVELS; ++i ) {
		total += current_detail_balance;
		current_detail_balance *= settings::LOD_LEVEL_DISTANCE_RATIO;
	}
	double sect{ ( m_camera->get_far_plane() - m_camera->get_near_plane() ) / total };
	double prev_pos{ m_camera->get_near_plane() };
	current_detail_balance = 1.0f;
	for( unsigned int i=0; i < settings::NUMBER_OF_LOD_LEVELS; ++i ) {
		// @todo why is this inverted ?
		m_visibility_ranges[settings::NUMBER_OF_LOD_LEVELS - i - 1] = prev_pos + sect * current_detail_balance;
		prev_pos = m_visibility_ranges[settings::NUMBER_OF_LOD_LEVELS - i - 1];
		current_detail_balance *= settings::LOD_LEVEL_DISTANCE_RATIO;
	}
	prev_pos = m_camera->get_near_plane();
	for( unsigned int i=0; i < settings::NUMBER_OF_LOD_LEVELS; ++i ) {
		unsigned int index{ settings::NUMBER_OF_LOD_LEVELS - i - 1 };
		m_morph_end[i] = m_visibility_ranges[index];
		m_morph_start[i] = prev_pos + ( m_morph_end[i] - prev_pos ) * settings::MORPH_START_RATIO;
		prev_pos = m_morph_start[i];
	}
	if( settings::DEBUG_OUTPUT_MORPH_LEVELS )
		debug_output_morph_levels();
}

void lod_selection::reset() {
	m_selection_count = 0;
	m_max_selected_lod_level = 0;
	m_sort_by_distance = settings::SORT_SELECTION;
	m_min_selected_lod_level = settings::NUMBER_OF_LOD_LEVELS-1;
	m_stop_at_level = settings::NUMBER_OF_LOD_LEVELS-1;
}

static inline int compareCloserFirst( const void *arg1, const void *arg2 ) {
	const lod_selection::selected_node *a = (const lod_selection::selected_node *)arg1;
	const lod_selection::selected_node *b = (const lod_selection::selected_node *)arg2;
	return a->min_distance_to_camera > b->min_distance_to_camera;
}

// sort by tile index and distance
void lod_selection::set_distances_and_sort() {
	if( m_sort_by_distance )
		std::qsort( m_selected_nodes, m_selection_count, sizeof( *m_selected_nodes ), compareCloserFirst );
}

void lod_selection::print_selection() const {
	std::ostringstream s;
	for( unsigned int i = 0; i < m_selection_count; ++i ) {
		const selected_node *n = &m_selected_nodes[i];
		omath::daabb box; n->p_node->get_world_aabb(box);
		s << "Selected node #" << i << " lod level " << n->lod_level <<" BB " <<
			box << "; distance from cam " << n->min_distance_to_camera << '\n';
	}
	logbook::log_msg( logbook::TERRAIN, logbook::INFO, s.str() );
}

static inline double cdlod_lerp(double const &a, double const &b, double const &f ) {
	return a + (b-a)*f;
}

const omath::vec4 lod_selection::get_morph_consts( const unsigned int lodLevel ) const {
	const double mStart{ m_morph_start[lodLevel] };
	double mEnd{ m_morph_end[lodLevel] };
	const double errorFudge{ 0.01f };
	mEnd = cdlod_lerp( mEnd, mStart, errorFudge );
	const double d{ mEnd - mStart };
	return omath::vec4{ mStart, 1.0f / d, mEnd / d, 1.0f / d };
}

bool lod_selection::selected_node::is_vis_dist_too_small() const {
	return (lod_level & 0x80000000) != 0;
}

void lod_selection::debug_output_morph_levels() const {
	std::ostringstream s;
	s << "Lod levels and ranges: lvl/range/start/end ";
	for( unsigned int i=0; i < settings::NUMBER_OF_LOD_LEVELS; ++i ) {
		s << "\n\t" << i << '/' << m_visibility_ranges[settings::NUMBER_OF_LOD_LEVELS - i - 1] <<
				'/' << m_morph_start[i] << '/' << m_morph_end[i];
	}
	logbook::log_msg( logbook::TERRAIN, logbook::INFO, s.str() );
}

}
