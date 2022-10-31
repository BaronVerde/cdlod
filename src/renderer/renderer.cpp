
#include "applications/cdlod/terrain_renderer.h"
#include "base/logbook.h"
#include "base/glfw_window.h"
#include "scene/scene.h"
#include "framebuffer.h"
#include "applications/ui_overlay/ui_overlay.h"
#include "renderer/renderer.h"
#include "applications/camera/camera.h"
#include "applications/sky_box/sky_box.h"

namespace orf_n {

renderer::renderer( bool debug ) : m_debug{ debug } {}

renderer::~renderer() {}

void renderer::setupRenderer() {
	// create render window, camera object and basic UIOverlay
	m_window = new glfw_window( "orf-n", 1920, 1080, m_debug );
	// TODO Before we proceed, perform a basic look around.
	if( !check_environment() ) {
		std::string s{ "The graphics environment is unfit to run this program." };
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
		throw std::runtime_error{ s };
	}
	// Just to have a camera object. Applications may set position and target
	m_camera = new camera{ m_window, omath::dvec3{ 0.0, 0.0, 1.0 }, omath::dvec3{ 0.0, 0.0, 0.0 },
		omath::vec3{ 0.0f, 1.0f, 0.0f }, 1.0f, 1000.0f, camera::FIRST_PERSON };
	m_overlay = new ui_overlay( m_window );

	m_framebuffer = new framebuffer( m_window->get_width(), m_window->get_height() );
	m_framebuffer->addColorAttachment( GL_RGB8_SNORM, GL_COLOR_ATTACHMENT0 );
	m_framebuffer->addDepthAttachment( GL_DEPTH_COMPONENT32F );
	if( !m_framebuffer->isComplete() ) {
		std::string s{ "Error creating framebuffer." };
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
		throw std::runtime_error{ s };
	} else
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, "Framebuffer/renderbuffer created." );
	// Build the scene and set it up.
	m_scene = new scene( m_window, m_camera, m_overlay );
	m_scene->add_renderable( 1, std::make_shared<sky_box>() );
	//m_scene->add_renderable( 1, std::make_shared<icosphere_ellipsoid>( Ellipsoid::WGS84_ELLIPSOID, 7 ) );
	m_scene->add_renderable( 2, std::make_shared<terrain::terrain_renderer>());
	//m_scene->add_renderable( 1, std::make_shared<shadow_map>() );
}

void renderer::setup() const {
	m_scene->setup();
}

void renderer::render() {
	logbook::log_msg( orf_n::logbook::RENDERER, orf_n::logbook::INFO,
			"--- Entering main loop ---" );
	double lastFrame { 0.0 };
	uint64_t frameCounter { 0 };

	while( !glfwWindowShouldClose( m_window->get_window() ) ) {
		double currentFrame { glfwGetTime() };
		++frameCounter;
		m_delta_time = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glEnable(GL_FRAMEBUFFER_SRGB);
		// Render in the framebuffer first
		m_framebuffer->bind( GL_DRAW_FRAMEBUFFER );
		m_framebuffer->clear( omath::vec4{ 0.0f, 0.0f, 0.0f, 1.0f } );

		m_scene->prepareFrame();
		// Called after prepareFrame() because UIOverlay has to start a new frame.
		m_scene->get_camera()->update_moving(m_delta_time);

		m_scene->render(m_delta_time);
		m_scene->endFrame();
		glDisable(GL_FRAMEBUFFER_SRGB);

		// Blit framebuffer to default window framebuffer
		m_framebuffer->bind( GL_READ_FRAMEBUFFER );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
		// Clear to blue to distinguish between draw framebuffer; color::blue.
		glClearColor( 0.0f, 0.0f, 1.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glBlitFramebuffer(
				0, 0, m_window->get_width(), m_window->get_height(),
				0, 0, m_window->get_width(), m_window->get_height(),
				GL_COLOR_BUFFER_BIT, GL_NEAREST
		);

		glfwPollEvents();
		glfwSwapBuffers( m_scene->get_window()->get_window() );

	}
	logbook::log_msg( orf_n::logbook::RENDERER, orf_n::logbook::INFO,"--- Leaving main loop ---" );
}

void renderer::cleanup() const {
	m_scene->cleanup();
}

void renderer::cleanupRenderer() {
	delete m_framebuffer;
	delete m_scene;
	delete m_overlay;
	delete m_camera;
	delete m_window;
}

static bool is_extension_supported(const char *name) {
	GLint n=0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	for( GLint i=0; i<n; ++i ) {
		const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
		if( !strcmp(name, extension) )
			return true;
	}
	return false;
}

static void list_extensions() {
	GLint n=0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	for( GLint i=0; i<n; ++i )
		logbook::log_msg(logbook::RENDERER, logbook::INFO, (const char*)glGetStringi(GL_EXTENSIONS, i));
}

// static
bool renderer::check_environment() {
	GLint retval;
	glGetIntegerv( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &retval );
	std::string s="Maximum combined texture units: " + std::to_string( retval ) + '.';
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &retval );
	s = "Maximum texture size: " + std::to_string( retval ) + '.';
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );
	glGetIntegerv( GL_MAX_TEXTURE_BUFFER_SIZE, &retval );
	s = "Maximum texture buffer size: " + std::to_string( retval ) + '.';
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s );
	// Query extensions. FIXME RGTC compression is too lossy for heigtmaps.
	/*if( is_extension_supported("GL_ARB_texture_compression_rgtc"))
		logbook::log_msg(logbook::RENDERER,logbook::INFO,"Texture compression RGTC supported.");
	else
		logbook::log_msg(logbook::RENDERER,logbook::INFO,"Texture compression RGTC not supported, will not compress.");	*/
	return true;
}

}	// namespace
