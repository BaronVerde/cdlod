
/* Draws a sky_box from an vector of 6 files.
 * The files must come in sequence right, left, up, down, front, back image.
 * Default values in cpp file.
 * The sky_box must be the first to be drawn to be in the background.
 * Sequence of faces:
 * GL_TEXTURE_CUBE_MAP_POSITIVE_X 	Right	0
 * GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	Left	1
 * GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	Top		2
 * GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	Bottom	3
 * GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	Back	4
 * GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	Front	5 */

#pragma once

#include "renderer/program.h"
#include "scene/renderable.h"
#include "omath/vec3.h"
#include <memory>
#include <string>
#include <vector>

class sky_box : public orf_n::renderable {
public:
	sky_box( const std::vector<std::string> &files = {} );

	virtual ~sky_box();

	void setup() override final;

	void render(const double delta_time) override final;

	void cleanup() override final;

	void bind() const;
	void un_bind() const;

private:
	void create_texture();

	const GLuint SKYBOX_VERTEX_BUFFER_BINDING_INDEX = 12;

	GLuint m_texture=0;
	GLuint m_vao=0;
	GLuint m_vertex_buffer=0;
	std::vector<std::string> m_faces;

	std::unique_ptr<orf_n::program> m_program{ nullptr };

};
