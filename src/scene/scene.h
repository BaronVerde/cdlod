
#pragma once

#include "renderable.h"
#include <map>
#include <memory>

namespace orf_n {

class glfw_window;
class camera;
class ui_overlay;

class scene : public renderable {
public:
	scene( glfw_window *win, camera *cam, ui_overlay *ovl );

	virtual ~scene();

	virtual void setup() override;

	virtual void prepareFrame() override;

	virtual void render(const double delta_time) override;

	virtual void endFrame() override;

	virtual void cleanup() override;

	void add_renderable( unsigned int order, std::shared_ptr<renderable> renderable_object );

	glfw_window *get_window() const;

	camera *get_camera() const;

private:
	glfw_window *m_window{ nullptr };

	camera *m_camera{ nullptr };

	ui_overlay *m_overlay{ nullptr };

	// Renderables in order for rendering.
	std::map<unsigned int, std::shared_ptr<renderable>> m_ordered_renderables;

};

}
