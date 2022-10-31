
#pragma once

#include <cstdint>
#include "omath/vec2.h"
#include "omath/aabb.h"

namespace terrain {

class heightmap_source {
public:
	heightmap_source() {};
	virtual ~heightmap_source() {};

	virtual const omath::uvec2 &get_extent() const = 0;
	virtual float get_height_at(const unsigned int x,const unsigned int y) const = 0;
	// Return height as float values normalied_values * 65535.0f
	virtual omath::vec2 get_area_min_max_height(
			const unsigned int x, const unsigned int z, const unsigned int size_x, const unsigned int size_z
	) const = 0;
	virtual const omath::aabb &get_raster_aabb() const = 0;
	virtual const omath::aabb &get_world_aabb() const = 0;
	// TODO this is going to change
	virtual void bind() const = 0;

};

}
