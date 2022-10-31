
#pragma once

#include <string>
#include <vector>
#include "glad/glad.h"

namespace orf_n {

class module {
public:
	/* In the following order:
	 * GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER,
	 * GL_GEOMETRY_SHADER, or GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER
	 * @param filename Pathname to shader. */
	module( const GLenum stage, const std::string &filename );

	module( const GLenum stage, const char** shader_source );

	module( const GLenum stage, const GLuint shader );

	virtual ~module();

	GLuint get_shader() const;

	bool is_shader() const;

	const std::string &get_filename() const;

private:
	GLuint m_shader;

	std::string m_filename;

	GLenum m_stage;

	bool m_is_shader;

	// Load a single standard glsl shader source file
	static std::vector<GLchar> load_shader_source( const std::string& filename );

	// Loads a source file. Scans for #include and loads the files named afetr that
	// Do not use <> or "" to bracket the filename, just plain whitespace !
	// Do not set clear_code to false if you don't want the code of the last loaded file.
	static std::vector<GLchar> parse_shader_source_file( const std::string& filename, bool clear_code = true );

	bool compile_shader( const char** shader_source );

};

}
