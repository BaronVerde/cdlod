
#pragma once

#include "omath/mat4.h"
#include <string>

namespace orf_n {

class scene;

/* Abstract class for the life cycle of a renderable object with 5 stages.
 * setup(), render() and cleanup() must be overridden. */
class renderable {
public:
	renderable( const std::string &name );

	virtual ~renderable();

	virtual const std::string &get_name() const final;

	virtual void set_scene( scene *scene ) final;

	virtual void setup() = 0;

	virtual void prepareFrame();

	virtual void render(const double delta_time) = 0;

	virtual void endFrame();

	virtual void cleanup() = 0;

	scene *m_scene;

	std::string m_name;

};

}
