
#pragma once

#include "renderer/program.h"
#include "omath/aabb.h"
#include "renderer/color.h"

namespace terrain {

class aabb_drawing {
public:

	static aabb_drawing &getInstance();
	void setup();
	void cleanup();
	void bind() const;
	void draw( const omath::daabb &bb, const orf_n::color_t &color ) const;
	const orf_n::program *getProgramPtr() const;

private:

	const GLuint VERTEX_ATTRIBUTE_LOCATION=0;

	aabb_drawing() = default;
	aabb_drawing( const aabb_drawing & ) = delete;
	void operator=( const aabb_drawing & ) = delete;

	virtual ~aabb_drawing();

	std::unique_ptr<orf_n::program> m_shaderDebug{ nullptr };
	GLuint m_vertex_array;
	GLuint m_vertex_buffer;
	GLuint m_index_buffer;

	bool m_set_up{false};

};

}
