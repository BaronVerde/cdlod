
#include "gridmesh.h"
#include "base/logbook.h"
#include "omath/vec3.h"
#include "settings.h"
#include <vector>

namespace terrain {

gridmesh::gridmesh( const unsigned int dimension ) : m_dimension{ dimension } {
	unsigned int num_vertices = ( m_dimension + 1 ) * ( m_dimension + 1 );
	m_number_of_indices = m_dimension * m_dimension * 2 * 3;

	std::vector<omath::vec3> vertices( num_vertices );
	unsigned int vert_dim = m_dimension + 1;
	for(unsigned int y = 0; y < vert_dim; ++y )
		for(unsigned int x = 0; x < vert_dim; ++x )
			vertices[x + vert_dim * y] =
					omath::vec3{ float(x) / float(m_dimension), 0.0f, float(y) / float(m_dimension) };
	glCreateVertexArrays( 1, &m_vertex_array );
	glCreateBuffers( 1, &m_vertex_buffer );
	glNamedBufferData( m_vertex_buffer, vertices.size() * sizeof(omath::vec3), vertices.data(), GL_STATIC_DRAW );
	glVertexArrayVertexBuffer( m_vertex_array, settings::GRIDMESH_VERTEX_BUFFER_BINDING_INDEX, m_vertex_buffer, 0, sizeof(omath::vec3) );
	glVertexArrayAttribBinding( m_vertex_array, 0, settings::GRIDMESH_VERTEX_BUFFER_BINDING_INDEX );
	glVertexArrayAttribFormat( m_vertex_array, 0, 3, GL_FLOAT, GL_FALSE, 0 );
	glEnableVertexArrayAttrib( m_vertex_array, 0 );
	std::vector<GLuint> indices( m_number_of_indices );
	GLuint index = 0;
	GLuint halfD = vert_dim / 2;
	m_numberOfSubmeshIndices = halfD * halfD * 6;
	//Top Left
	for( GLuint y = 0; y < halfD; ++y ) {
		for( GLuint x = 0; x < halfD; ++x ) {
			indices[index++] = x + vert_dim * y;
			indices[index++] = x + vert_dim * (y + 1);
			indices[index++] = (x + 1) + vert_dim * y;
			indices[index++] = (x + 1) + vert_dim * y;
			indices[index++] = x + vert_dim * (y + 1);
			indices[index++] = (x + 1) + vert_dim * (y + 1);
		}
	}
	m_endIndexTopLeft = index;
	//Top Right
	for( GLuint y = 0; y < halfD; ++y ) {
		for( GLuint x = halfD; x < m_dimension; ++x ) {
			indices[index++] = x + vert_dim * y;
			indices[index++] = x + vert_dim * (y + 1);
			indices[index++] = (x + 1) + vert_dim * y;
			indices[index++] = (x + 1) + vert_dim * y;
			indices[index++] = x + vert_dim * (y + 1);
			indices[index++] = (x + 1) + vert_dim * (y + 1);
		}
	}
	m_endIndexTopRight = index;
	//Bottom Left
	for( GLuint y = halfD; y < m_dimension; ++y ) {
		for( GLuint x = 0; x < halfD; ++x ) {
			indices[index++] = x + vert_dim * y;
			indices[index++] = x + vert_dim * (y + 1);
			indices[index++] = (x + 1) + vert_dim * y;
			indices[index++] = (x + 1) + vert_dim * y;
			indices[index++] = x + vert_dim * (y + 1);
			indices[index++] = (x + 1) + vert_dim * (y + 1);
		}
	}
	m_endIndexBottomLeft = index;
	//Bottom Right
	for( GLuint y = halfD; y < m_dimension; ++y ) {
		for( GLuint x = halfD; x < m_dimension; ++x ) {
			indices[index++] = x + vert_dim * y;
			indices[index++] = x + vert_dim * (y + 1);
			indices[index++] = (x + 1) + vert_dim * y;
			indices[index++] = (x + 1) + vert_dim * y;
			indices[index++] = x + vert_dim * (y + 1);
			indices[index++] = (x + 1) + vert_dim * (y + 1);
		}
	}
	m_endIndexBottomRight = index;
	glCreateBuffers( 1, &m_index_buffer );
	glNamedBufferData( m_index_buffer, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW );
	glVertexArrayElementBuffer( m_vertex_array, m_index_buffer );
	if( (GLsizei)indices.size() != m_number_of_indices )
		orf_n::logbook::log_msg(
			orf_n::logbook::TERRAIN, orf_n::logbook::WARNING,"Number of gridmesh indices unequals precalculated number."
		);
	orf_n::logbook::log_msg(
		orf_n::logbook::TERRAIN, orf_n::logbook::INFO,"Gridmesh dimension "+std::to_string( m_dimension )+" created."
	);
}

void gridmesh::bind() const {
	glBindVertexArray( m_vertex_array );
}

gridmesh::~gridmesh() {
	glDisableVertexArrayAttrib( m_vertex_array, 0 );
	glDeleteBuffers( 1, &m_index_buffer );
	glDeleteBuffers( 1, &m_vertex_buffer );
	glDeleteVertexArrays( 1, &m_vertex_array );
	orf_n::logbook::log_msg( orf_n::logbook::TERRAIN, orf_n::logbook::INFO,
			"Gridmesh dimension " + std::to_string( m_dimension ) + " destroyed." );
}

unsigned int gridmesh::getDimension() const {
	return m_dimension;
}

unsigned int gridmesh::getEndIndexTL() const {
	return m_endIndexTopLeft;
}

unsigned int gridmesh::getEndIndexTR() const {
	return m_endIndexTopRight;
}

unsigned int gridmesh::getEndIndexBL() const {
	return m_endIndexBottomLeft;
}

unsigned int gridmesh::getEndIndexBR() const {
	return m_endIndexBottomRight;
}

unsigned int gridmesh::getNumberOfSubMeshIndices() const {
	return m_numberOfSubmeshIndices;
}

GLsizei gridmesh::get_number_indices() const {
	return m_number_of_indices;
}

}
