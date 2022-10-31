
#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "event_handler.h"
#include <memory>
#include <string>

namespace orf_n {

class glfw_window {
public:
    glfw_window( const std::string &title, const int width, const int height, bool debug = false );

    virtual ~glfw_window();

    GLFWwindow *get_window() const;

    const event_handler *get_event_handler() const;

    // std::string getTitle() const { return m_title; }

    int get_width() const;

    void set_width( int width );

    void set_height( int height );

    int get_height() const;

    void set_title( const std::string &title );

    void set_should_close() const;

    void toggle_input_mode();

    bool is_cursor_disabled() const;

    glfw_window &operator=( const glfw_window &eh ) = delete;

    glfw_window( const glfw_window &eh ) = delete;

    bool get_v_sync() const;

    void set_v_sync( bool v );

    void set_damaged( bool damaged );

    bool get_damaged() const;

private:
    GLFWwindow* m_window{nullptr};

    std::unique_ptr<event_handler> m_event_handler{nullptr};

    std::string m_title;

    int m_width;

    int m_height;

    // initial mouse cursor state, see ctor
    bool m_cursorDisabled{ false };

    bool m_isDamaged{ false };

    bool m_debug{true};

    bool m_vsync{true};

    static void errorCallback( int error, const char *msg );

    static void APIENTRY glDebugOutput( GLenum source, GLenum type, GLuint id, GLenum severity,
    		GLsizei length, const GLchar *message, const void *userParam );

};	// class glfw_window

}	// namespace
