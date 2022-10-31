
#include <applications/cdlod/terrain_renderer.h>
#include "node.h"
#include "quadtree.h"
#include "heightmap.h"
#include "settings.h"
#include "base/logbook.h"
#include <sstream>

using namespace orf_n;

namespace terrain {

quadtree::quadtree( const terrain_renderer *const lod ) :
		m_lod{ lod } {
	// TODO: Checks.
	if( m_lod->get_heightmap()->get_extent().x > 65535 || m_lod->get_heightmap()->get_extent().y > 65535 ) {
		std::string s{ "Heightmap too large (>65535) for the quad tree." };
		logbook::log_msg( logbook::TERRAIN, logbook::ERROR, s );
		throw std::runtime_error( s );
	}
}

bool quadtree::create() {
	// Determine how many nodes will we use, and the size of the top (root) tree node.
	unsigned int size_x = settings::RASTER_MAX.x - settings::RASTER_MIN.x;
	unsigned int size_z = settings::RASTER_MAX.z - settings::RASTER_MIN.z;
	unsigned int totalNodeCount = 0;
	m_topNodeSize = settings::LEAF_NODE_SIZE;
	for( unsigned int i = 0; i < settings::NUMBER_OF_LOD_LEVELS; ++i ) {
		if( i != 0 )
			m_topNodeSize *= 2;
		unsigned int nodeCountX = ( size_x - 1 ) / m_topNodeSize + 1;
		unsigned int nodeCountZ = ( size_z- 1 ) / m_topNodeSize + 1;
		totalNodeCount += nodeCountX * nodeCountZ;
	}
	// Initialize the tree memory, create tree nodes, and extract min/max Ys (heights)
	m_allNodes = new node[totalNodeCount];
	unsigned int nodeCounter = 0;
	m_topNodeCountX = ( size_x - 1 ) / m_topNodeSize + 1;
	m_topNodeCountZ = ( size_z - 1 ) / m_topNodeSize + 1;
	m_topLevelNodes = new node**[m_topNodeCountZ];
	for( unsigned int z=0; z < m_topNodeCountZ; ++z ) {
		m_topLevelNodes[z] = new node*[m_topNodeCountX];
		for( unsigned int x{ 0 }; x < m_topNodeCountX; ++x ) {
			m_topLevelNodes[z][x] = &m_allNodes[nodeCounter];
			nodeCounter++;
			m_topLevelNodes[z][x]->create(
					x * m_topNodeSize, z * m_topNodeSize,m_topNodeSize, 0, m_lod, m_allNodes, nodeCounter
			);
		}
	}
	m_nodeCount = nodeCounter;
	if( m_nodeCount != totalNodeCount ) {
		std::ostringstream s;
		s << "Node counter (" << m_nodeCount << ") does not equal pre-calculated node count ("<<totalNodeCount<< ").";
		logbook::log_msg( logbook::TERRAIN, logbook::ERROR, s.str() );
		throw std::runtime_error( s.str() );
	}
	// Debug output
	std::ostringstream s;
	// Quad tree summary
	float sizeInMemory = (float)m_nodeCount * ( sizeof( node ) + sizeof( omath::aabb ) );
	s << "Quad tree created " << m_nodeCount << " Nodes; size in memory: " << ( sizeInMemory / 1024.0f ) <<
		"kB.\n\t" << m_topNodeCountX << '*' << m_topNodeCountZ << " top nodes.";
	logbook::log_msg( logbook::TERRAIN, logbook::INFO, s.str() );
	// Debug: List of all Nodes
	if( settings::DEBUG_OUTPUT_TREE_NODES )
		debug_output_nodes();
	return true;
}

void quadtree::cleanup() {
	if( m_allNodes != nullptr ) {
		delete[] m_allNodes;
		m_allNodes = nullptr;
	}
	if( m_topLevelNodes != nullptr ) {
		for( unsigned int y{ 0 }; y < m_topNodeCountZ; ++y ) {
			delete[] m_topLevelNodes[y];
			m_topLevelNodes[y] = nullptr;
		}
		delete[] m_topLevelNodes;
		m_topLevelNodes = nullptr;
	}
}

quadtree::~quadtree() {
	cleanup();
	logbook::log_msg( logbook::TERRAIN, logbook::ERROR, "Quadtree deleted." );
}

const node *quadtree::getNodes() const {
	return m_allNodes;
}

unsigned int quadtree::getNodeCount() const {
	return m_nodeCount;
}

void quadtree::lodSelect( lod_selection *lodSelection ) const {
	for( unsigned int z{ 0 }; z < m_topNodeCountZ; ++z )
		for( unsigned int x{ 0 }; x < m_topNodeCountX; ++x )
			m_topLevelNodes[z][x]->lod_select( lodSelection, false );
}

void quadtree::debug_output_nodes() const {
	std::ostringstream s;
	for( unsigned int i=0; i < m_nodeCount; ++i ) {
		s.str( std::string() );
		node *n{ &m_allNodes[i] };
		omath::daabb box; n->get_world_aabb(box);
		s << "Node " << i << " Level " << n->get_level() << " BB " << box;
		if( n->is_leaf() )
			s << "; is leaf node.";
		else {
			s << "; child lvls: " << n->get_tl()->get_level() << '/' << n->get_tr()->get_level() << '/' <<
			n->get_bl()->get_level() << '/' << n->get_br()->get_level();
		}
		logbook::log_msg( logbook::TERRAIN, logbook::INFO, s.str() );
	}
}

}
