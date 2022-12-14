
#include "glfw_window.h"
#include "logbook.h"
#include <iostream>

namespace orf_n {

glfw_window::glfw_window( const std::string &title, const int width, const int height, bool debug ) :
		m_title( title ), m_width( width ), m_height( height ), m_debug( debug ) {
	// error callbacks
	glfwSetErrorCallback( errorCallback );
	if( GL_TRUE != glfwInit() )
		throw std::runtime_error( "Error initialising glfw" );
	// Window and context; we want OpenGL 4.5
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
	if( m_debug )
		glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
	glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_API );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
	glfwWindowHint( GLFW_SAMPLES, 0 );
	glfwWindowHint( GLFW_DEPTH_BITS, 32 );
	m_window = glfwCreateWindow( width, height, title.c_str(), nullptr, nullptr );
	if( NULL == m_window )
		throw std::runtime_error( "Could not create glfw window\n" );
	glfwMakeContextCurrent( m_window );
	if( !gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) )
		throw std::runtime_error( "Error initialising glad" );
	logbook::log_msg( logbook::WINDOW, logbook::INFO, "OpenGL 4.5 context created." );
	// swap interval for current context, start in vsync mode
	glfwSwapInterval( 1 );
	m_vsync = true;

	// event_handler
	m_event_handler = std::make_unique<event_handler>( this );
	glfwSetWindowUserPointer( m_window, m_event_handler.get() );

	if( m_debug ) {
		// Initialize debug output
		GLint flags;
		glGetIntegerv( GL_CONTEXT_FLAGS, &flags );
		if( flags & GL_CONTEXT_FLAG_DEBUG_BIT ) {
			glEnable( GL_DEBUG_OUTPUT );
			// Envoke callback directly in case of error
		    glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
		    glDebugMessageCallback( glDebugOutput, nullptr );
		    glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
		    logbook::log_msg( logbook::WINDOW, logbook::INFO, "Debug context created." );
		} else
			logbook::log_msg( logbook::WINDOW, logbook::WARNING,
					"Debug context not created. Continuing without debug messages." );
	}

	// Ensure we can capture key events
	glfwSetInputMode( m_window, GLFW_STICKY_KEYS, GLFW_TRUE );
	// initially disable cursor for camera control
	glfwSetInputMode( m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
	m_cursorDisabled = false;

}	// ctor

// virtual
glfw_window::~glfw_window() {
	glfwSetWindowUserPointer( m_window, NULL );
	glfwDestroyWindow( m_window );
	m_window=nullptr;
	glfwTerminate();
}	// dtor

void glfw_window::toggle_input_mode() {
	if( m_cursorDisabled )
		glfwSetInputMode( m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
	else
		glfwSetInputMode( m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
	m_cursorDisabled = !m_cursorDisabled;
}

void glfw_window::set_v_sync( bool v ) {
	m_vsync = v;
	glfwSwapInterval( m_vsync ? 1 : 0 );
}

GLFWwindow* glfw_window::get_window() const {
	return m_window;
}

const event_handler *glfw_window::get_event_handler() const {
	return m_event_handler.get();
}

int glfw_window::get_width() const { return m_width; }

int glfw_window::get_height() const { return m_height; }

void glfw_window::set_width( int width ) { m_width = width; }

void glfw_window::set_height( int height ) { m_height = height; }

void glfw_window::set_title( const std::string &title ) {
	m_title = title;
   	glfwSetWindowTitle( m_window, m_title.c_str() );
}

void glfw_window::set_damaged( bool damaged ) {
	m_isDamaged = damaged;
}

bool glfw_window::get_damaged() const {
	return m_isDamaged;
}

void glfw_window::set_should_close() const {
	glfwSetWindowShouldClose( m_window, GL_TRUE );
}

bool glfw_window::is_cursor_disabled() const { return m_cursorDisabled; }

bool glfw_window::get_v_sync() const { return m_vsync; }

// static
void glfw_window::errorCallback( int error, const char *msg ) {
	std::string s;
	s = " [" + std::to_string(error) + "] " + msg;
	logbook::log_msg( logbook::WINDOW, logbook::ERROR, s );
	throw std::runtime_error( "GLFW window error, see logbook" );
}

// static
void APIENTRY glfw_window::glDebugOutput( GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar *message, const void *userParam ) {

	// Ignore non-significant error/warning codes
	if( /*id == 131169 ||	// framebuffer storage allocation */
		id == 131185 /*||	// buffer memory usage
		id == 131218 ||	// shader being recompiled
		id == 131204 ||
		// shader compiler debug messages
		id == 6 || id == 7 || id == 8 || id == 9 || id == 10 ||
		id == 11 || id == 12 || id == 13 || id == 14*/ )
		return;

    std::string s = "Debug message (" + std::to_string(id) + "): ";
    switch( source ) {
        case GL_DEBUG_SOURCE_API:             s += "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   s += "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: s += "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     s += "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     s += "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           s += "Source: Other"; break;
    }
    s += "; ";

    switch( type ) {
        case GL_DEBUG_TYPE_ERROR:               s += "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: s += "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  s += "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         s += "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         s += "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              s += "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          s += "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           s += "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               s += "Type: Other"; break;
    }
    s += "; ";

    switch( severity ) {
        case GL_DEBUG_SEVERITY_HIGH:         s += "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       s += "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          s += "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: s += "Severity: notification"; break;
    }
    s = s + "; " + message+" ("+std::to_string(length)+").";
    if(userParam)
    	s+="; Additional user data not printed.";

    logbook::log_msg( logbook::RENDERER, logbook::LOG, s );
}	// glDebugOutput()

}	// namespace
