
#include "base/logbook.h"
#include "renderer/uniform.h"
#include "omath/mat4.h"
#include "aabb_drawing.h"
#include "settings.h"
#include "glad/glad.h"

using namespace orf_n;

namespace terrain {

//static
aabb_drawing &aabb_drawing::getInstance() {
	static aabb_drawing onceOnly;
	return onceOnly;
}

aabb_drawing::~aabb_drawing() {
	cleanup();
}

void aabb_drawing::setup() {
	if( m_set_up )
		logbook::log_msg( logbook::RENDERER, logbook::WARNING, "Debug drawing already set up." );
	else {
		// Load shaders
		std::vector<std::shared_ptr<orf_n::module>> modules;
		modules.push_back(
				std::make_shared<module>( GL_VERTEX_SHADER, "src/applications/cdlod/pass_through.vert.glsl" )
		);
		modules.push_back(
				std::make_shared<module>( GL_FRAGMENT_SHADER, "src/applications/cdlod/pass_through.frag.glsl" )
		);
		m_shaderDebug = std::make_unique<program>( modules );
		// Vertex array and indices for an aabb
		const std::vector<omath::vec3> box_vertices {
			{ -0.5f, -0.5f, -0.5f },
			{  0.5f, -0.5f, -0.5f },
			{  0.5f,  0.5f, -0.5f },
			{ -0.5f,  0.5f, -0.5f },
			{ -0.5f, -0.5f,  0.5f },
			{  0.5f, -0.5f,  0.5f },
			{  0.5f,  0.5f,  0.5f },
			{ -0.5f,  0.5f,  0.5f }
		};
		// Box vertex array.
		glCreateVertexArrays(1,&m_vertex_array);
		glCreateBuffers(1,&m_vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER,m_vertex_buffer);
		//glNamedBufferData( m_vertex_buffer, boxVertices.size()*sizeof(omath::vec3), boxVertices.data(), GL_STATIC_DRAW );
		glNamedBufferStorage( m_vertex_buffer, box_vertices.size()*sizeof(omath::vec3), box_vertices.data(), 0 );
		// Shortcut
		GLuint vb = terrain::settings::AABB_DRAWING_VERTEX_BUFFER_BINDING_INDEX;
		glVertexArrayVertexBuffer(
				m_vertex_array, vb, m_vertex_buffer, 0, sizeof( omath::vec3 )
		);
		glVertexArrayAttribFormat( m_vertex_array, VERTEX_ATTRIBUTE_LOCATION, 3, GL_FLOAT, GL_FALSE, 0 );
		glVertexArrayAttribBinding( m_vertex_array, VERTEX_ATTRIBUTE_LOCATION, vb );
		glEnableVertexArrayAttrib( m_vertex_array, VERTEX_ATTRIBUTE_LOCATION );
		// Box index buffer.
		const std::vector<GLuint> indices {
			0, 4, 0, 1, 5, 1, 2, 6, 2, 3, 7, 6, 5, 4, 7, 3
		};
		glCreateBuffers(1,&m_index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_index_buffer);
		//glNamedBufferData( m_index_buffer, boxLoopIndices.size()*sizeof(GLuint), boxLoopIndices.data(), GL_STATIC_DRAW );
		glNamedBufferStorage( m_index_buffer, indices.size()*sizeof(GLuint), indices.data(), 0 );
		logbook::log_msg( logbook::RENDERER, logbook::INFO, "Debug drawing set up." );
		m_set_up = true;
	}
}

void aabb_drawing::cleanup() {
	if( m_set_up ) {
		m_shaderDebug.reset();
		glDeleteVertexArrays(1,&m_vertex_array);
		glDeleteBuffers(1,&m_vertex_buffer);
		glDeleteBuffers(1,&m_index_buffer);
		m_set_up = false;
	}
}

void aabb_drawing::draw( const omath::aabb &bb, const color_t &color ) const {
	const omath::mat4 m = omath::mat4();
	const omath::vec3 c = bb.get_center();
	const omath::mat4 modelM{ omath::translate( m, c ) * omath::scale( m, omath::vec3{ bb.get_size() } ) };
	orf_n::set_uniform( m_shaderDebug->get_program(), "modelMatrix", modelM );
	orf_n::set_uniform( m_shaderDebug->get_program(), "debugColor", color );
	glBindVertexArray(m_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER,m_vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_index_buffer);
	glDrawElements( GL_LINE_LOOP, 16, GL_UNSIGNED_INT, NULL );
}

const program *aabb_drawing::getProgramPtr() const {
	return m_shaderDebug.get();
}

}
