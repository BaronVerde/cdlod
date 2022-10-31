
#pragma once

#include "omath/vec2.h"
#include "glad/glad.h"
#include <string>

namespace terrain {

class heightmap {
public:

	static constexpr GLuint HEIGHTMAP_TEXTURE_UNIT{0};
	typedef enum : unsigned int {
		B8, B16
	} bit_depth;
	heightmap( const std::string &filename, const bit_depth depth = B16 );
	virtual ~heightmap();
	void bind() const;
	void unbind() const;
	const omath::uvec2 &get_extent() const;
	const GLuint &get_texture() const;
	// Returns the real world height value at coords (normalized * 65535.0f).
	float get_height_at( const unsigned int x, const unsigned int y ) const;
	// Returns min/max values in the world range of 0.0f..65535.0f
	omath::vec2 get_min_max_height_area(
			const unsigned int x, const unsigned int z, const unsigned int w, const unsigned int h
	) const;

private:
	std::string m_filename{ "" };
	// Height values, normalized from uint16_t to 0..1 over the range of uint16_t.
	uint16_t *m_height_values=nullptr;
	GLuint m_texture{ 0 };
	/* Height/width of texture file in pixels.
	 * Integer because opengl expects integer in texture addressing and for loops compare to <=0 ... */
	omath::uvec2 m_extent{ 0, 0 };
	// Depth of the texture, factors are derivee from that.
	bit_depth m_bit_depth{ B16 };
	const bit_depth &get_depth() const;

};

}
