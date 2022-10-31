
#pragma once

#include "scene/renderable.h"
#include "glad/glad.h"
#include <imgui/imgui.h>

namespace orf_n {

class program;
class glfw_window;

class ui_overlay : public renderable, event_handler {
public:
	ui_overlay( glfw_window *win );

	virtual ~ui_overlay();

	void setup() override final;

	void prepareFrame() override final;

	void render(const double delta_time) override final;

	void cleanup() override final;

	ui_overlay &operator=( const ui_overlay &eh ) = delete;

	ui_overlay( const ui_overlay &eh ) = delete;

private:
	/* Shader unit the font texture is bound to and not rebound !
	 * When creating textures, leave this texture binding point untouched. */
	static constexpr GLint FONT_TEXTURE_UNIT=20;

	bool m_showUI{ true };

	glfw_window *m_window{nullptr};

	program *m_shader{nullptr};

	GLuint m_fontTexture;

	// Global alpha for the UI. Must not be 0 or UI becomes invisible !
	float m_uiAlpha{ 0.6f };

	int m_AttribLocationTex;
	int m_AttribLocationPosition;
	int m_AttribLocationUV;
	int m_AttribLocationColor;

	GLuint m_vboHandle;

	GLuint m_elementsHandle;

	virtual bool on_mouse_move( float x, float y ) override;

	virtual bool on_mouse_button( int button, int action, int mods ) override;

	virtual bool on_mouse_scroll( float xOffset, float yOffset ) override;

	virtual bool on_key_pressed( int key, int scancode, int action, int mods ) override;

	virtual bool on_char( unsigned int code ) override;

};

}
