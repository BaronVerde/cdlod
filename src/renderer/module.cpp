
#include "module.h"
#include <fstream>
#include "base/logbook.h"
#include <iostream>
#include <cstdio>

namespace orf_n {

module::module( const GLenum stage, const std::string &filename ) :
		m_filename( filename ), m_stage( stage ), m_is_shader( false ) {
	std::vector<GLchar> source=load_shader_source( m_filename );
	if( !source.empty() ) {
		m_shader = glCreateShader( stage );
		const GLchar *x{ source.data() };
		if( compile_shader( &x ) )
			m_is_shader = true;
	} else
		logbook::log_msg(logbook::SHADER, logbook::ERROR, "Shader '"+m_filename+"' not loaded.");
}

module::module( const GLenum stage, const char** shader_source ) : m_stage{stage} {
	m_shader = glCreateShader( stage );
	if( compile_shader( shader_source ) )
		m_is_shader = true;
}

module::module( const GLenum stage, const GLuint shader ) :
		m_shader{shader}, m_stage{stage} {
	m_is_shader = glIsShader( shader ) ? true : false;
}

module::~module() {
	if( m_is_shader )
		glDeleteShader( m_shader );
}

GLuint module::get_shader() const {
	return m_shader;
}

bool module::is_shader() const {
	return m_is_shader;
}

const std::string &module::get_filename() const {
	return m_filename;
}

bool module::compile_shader( const char** shader_source ) {
	// Source assumed to be null-terminated
	glShaderSource( m_shader, 1, shader_source, NULL );
	glCompileShader( m_shader );
	GLint compiled;
	glGetShaderiv( m_shader, GL_COMPILE_STATUS, &compiled );
	if( !compiled ) {
		GLsizei len;
		glGetShaderiv( m_shader, GL_INFO_LOG_LENGTH, &len );
		GLchar *log = new GLchar[len+1];
		glGetShaderInfoLog( m_shader, len, &len, log );
		logbook::log_msg(logbook::SHADER, logbook::ERROR, "Shader '"+m_filename+"' compilation failed:\n"+log);
		delete [] log;
		glDeleteShader( m_shader );
		return false;
	}
	return true;
}

// static
std::vector<GLchar> module::load_shader_source( const std::string& filename ) {
	std::vector<GLchar> buffer;
	std::ifstream file{ filename, std::ios::ate | std::ios::binary };
	if( file.is_open() ) {
		size_t fileSize{ (size_t)file.tellg() };
		buffer.resize(fileSize + 1);
		file.seekg(0);
		file.read( buffer.data(), fileSize );
		file.close();
		buffer[fileSize] = '\0';
	}
	return buffer;
}	// readFile()

// static
std::vector<GLchar> module::parse_shader_source_file( const std::string& filename, bool clear_code ) {
	// static to accumulate over recursive calls
	static std::vector<std::string> source_code;
	if( clear_code )
		source_code.clear();
	std::ifstream file{ filename, std::ios::in };
	if( file ) {
		std::string line;
		char found_filename[50];
		while( getline( file, line ) ) {
			if( 1 == sscanf( line.c_str(), "#include %49s", found_filename ) )
				parse_shader_source_file( found_filename, false );
			else
				source_code.push_back( line + '\n' );
		}
		file.close();
	}
	std::vector<GLchar> retval;
	for( const std::string& s : source_code )
		std::copy( s.begin(), s.end(), std::back_inserter( retval ) );
	return retval;
}

}
