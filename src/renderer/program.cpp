
#include <base/logbook.h>
#include <renderer/program.h>
#include <fstream>
#include <stdexcept>
#include <sstream>

namespace orf_n {

program::program( const std::vector<std::shared_ptr<module>> &modules ) {
    if( modules.size() == 0 ) {
    	std::string s{ "No shader modules specified for program." };
    	logbook::log_msg( logbook::SHADER, logbook::ERROR, s );
    	throw std::runtime_error( s );
    }
    m_program = glCreateProgram();
    if( !glIsProgram( m_program ) ) {
    	std::string s{ "Error creating shader program. Is not a program." };
    	logbook::log_msg( logbook::SHADER, logbook::ERROR, s );
    	throw std::runtime_error( s );
    }
    for( const std::shared_ptr<module> &m : modules ) {
        glAttachShader( m_program, m->get_shader() );
        std::ostringstream s;
    }
    link();
    std::ostringstream s;
    s << "Shader program #" << m_program << " linked. Ready for use.";
    logbook::log_msg( logbook::SHADER, logbook::INFO, s.str() );
}

program::~program() {
	glDeleteProgram( m_program );
	logbook::log_msg(
			logbook::SHADER, logbook::INFO,
			"Shader program #" + std::to_string( m_program ) + " destroyed."
	);
}

GLuint program::get_program() const {
	return m_program;
}

void program::use() const {
	glUseProgram( m_program );
}

void program::un_use() const {
	glUseProgram( 0 );
}

void program::link() const {
	glLinkProgram( m_program );
	GLint linked;
	glGetProgramiv( m_program, GL_LINK_STATUS, &linked );
	if( !linked ) {
		GLsizei len;
		glGetProgramiv( m_program, GL_INFO_LOG_LENGTH, &len );
		GLchar *log = new GLchar[len+1];
		glGetProgramInfoLog( m_program, len, &len, log );
		std::string s="Shader linkage failed: " + std::string(log);
		logbook::log_msg( logbook::SHADER, logbook::ERROR, s );
		delete [] log;
		glDeleteProgram( m_program );
		throw std::runtime_error( s );
	}

}

}	// namespace
