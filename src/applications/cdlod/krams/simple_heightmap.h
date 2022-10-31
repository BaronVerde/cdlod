
#pragma once

#include "heightmap_source.h"
#include "glad/glad.h"
#include "omath/vec2.h"
#include "omath/aabb.h"
#include <string>

namespace terrain {

class simple_heightmap : public heightmap_source {
public:
	simple_heightmap( const std::string &filename );
	virtual ~simple_heightmap();

	virtual const omath::uvec2 &get_size() const override;
	// The following methods return the real world height (normalized*65535.0f) value at coords
	virtual float get_height_at(unsigned int x,unsigned int y) const override;
	virtual omath::vec2 get_area_min_max_height(
			unsigned int x, unsigned int z, unsigned int size_x, unsigned int size_z
	) const override;
	virtual const omath::aabb &get_raster_aabb() const override;
	virtual const omath::aabb &get_world_aabb() const override;
	virtual void bind() const override;

private:
	std::string m_filename{""};
	// Real world height values, not normalized, normalized to 0..1 over the range of uint16_t
	// TODO: maybe use 16 bit values for better performance ?
	float *m_height_values_normalized{ nullptr };
	GLuint m_texture = 0;
	omath::uvec2 m_size;
	omath::aabb m_raster_aabb;
	omath::aabb m_world_aabb;

};

}
