
#pragma once

namespace orf_n {

class framebuffer;
class ui_overlay;
class scene;
class camera;
class glfw_window;

class renderer {
public:
	renderer( bool debug = true );
	virtual ~renderer();

	void setupRenderer();

	void setup() const;

	void render();

	void cleanup() const;

	void cleanupRenderer();

private:
	bool m_debug{true};
	double m_delta_time;

	scene *m_scene{ nullptr };

	glfw_window *m_window{ nullptr };

	camera *m_camera{ nullptr };

	framebuffer *m_framebuffer{ nullptr };

	ui_overlay *m_overlay{ nullptr };

	// A stub now. In future, suitability checks should be done here.
	bool check_environment();

};

}	// namespace
