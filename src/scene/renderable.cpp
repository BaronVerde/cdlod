
#include "base/logbook.h"
#include "renderable.h"

namespace orf_n {

renderable::renderable( const std::string &name ) : m_name(name) {}

renderable::~renderable() {}

const std::string &renderable::get_name() const {
	return m_name;
}

void renderable::set_scene( scene *scene ) {
	m_scene = scene;
}

void renderable::prepareFrame() {}

void renderable::endFrame() {}

}
