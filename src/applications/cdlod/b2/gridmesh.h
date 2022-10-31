
/* A rectangular, [0.0..1.0] clamped regular flat mesh. X and Z are the horizontal dimensions.
 * Y will be extruded by the heightmap. Index- and vertex buffer are built for element drawing.
 * Indices are givven for all 4 quadrants of the mesh. This is needed for the LOD rendering */

#pragma once

#include "glad/glad.h"

namespace terrain {

class gridmesh {
public:
	gridmesh(unsigned int dimension );
	virtual~gridmesh();
	unsigned int getDimension() const;
	unsigned int getEndIndexTL() const;
	unsigned int getEndIndexTR() const;
	unsigned int getEndIndexBL() const;
	unsigned int getEndIndexBR() const;
	unsigned int getNumberOfSubMeshIndices() const;
	GLsizei get_number_indices() const;
	void bind() const;

private:
	GLuint m_vertex_array;
	GLuint m_index_buffer;
	GLuint m_vertex_buffer;
	unsigned int m_dimension = 0;
	unsigned int m_endIndexTopLeft = 0;
	unsigned int m_endIndexTopRight = 0;
	unsigned int m_endIndexBottomLeft = 0;
	unsigned int m_endIndexBottomRight = 0;
	unsigned int m_numberOfSubmeshIndices = 0;
	GLsizei m_number_of_indices = 0;

};

}
