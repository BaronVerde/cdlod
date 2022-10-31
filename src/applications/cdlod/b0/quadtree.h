
#pragma once

namespace terrain {

class terrain_renderer;
class lod_selection;
class node;

// @todo: needs overwork. Much data is meaningless
class quadtree {
public:
	quadtree( const terrain_renderer *const lod );
	virtual ~quadtree();
	// Create the tree from settings raster size.
	bool create();
	void cleanup();
	const node *getNodes() const;
	unsigned int getNodeCount() const;
	// tile index is saved in selection list for sorting by tile and distance
	void lodSelect( lod_selection *lodSelectlion ) const;

private:
	unsigned int m_topNodeSize = 0;
	unsigned int m_topNodeCountX = 0;
	unsigned int m_topNodeCountZ=0;
	unsigned int m_nodeCount=0;
	node *m_allNodes=nullptr;
	node ***m_topLevelNodes=nullptr;
	const terrain_renderer *const m_lod=nullptr;

	void debug_output_nodes() const;

};

}
