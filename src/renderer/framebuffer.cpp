
#include "base/logbook.h"
#include "framebuffer.h"
#include "iostream"

namespace orf_n {

framebuffer::framebuffer( const unsigned int x, const unsigned int y ) :
		m_sizeX{ x }, m_sizeY{ y } {
	glCreateFramebuffers( 1, &m_framebuffer );
	if( GL_TRUE != glIsFramebuffer( m_framebuffer ) ) {
		std::string s{ "Error creating framebuffer object." };
		logbook::log_msg( logbook::RENDERER, logbook::INFO, s );
		throw std::runtime_error( s );
	}
	logbook::log_msg(
			logbook::RENDERER, logbook::INFO, "Created framebuffer object " + std::to_string( m_framebuffer )
	);
}

framebuffer::~framebuffer() {
	if( GL_TRUE == glIsRenderbuffer( m_depthAttachment ) )
		glDeleteRenderbuffers( 1, &m_depthAttachment );
	if( GL_TRUE == glIsRenderbuffer( m_colorAttachment ) )
		glDeleteRenderbuffers( 1, &m_colorAttachment );
	glDeleteFramebuffers( 1, &m_framebuffer );
	logbook::log_msg(
			logbook::RENDERER, logbook::INFO, "framebuffer object " + std::to_string( m_framebuffer ) + " destroyed."
	);
}

void framebuffer::addColorAttachment( const GLenum colorFormat, GLenum attachmentPoint ) {
	glCreateRenderbuffers( 1, &m_colorAttachment );
	if( GL_TRUE != glIsRenderbuffer( m_colorAttachment ) )
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, "Error creating color renderbuffer" );
	glNamedRenderbufferStorage( m_colorAttachment, colorFormat, m_sizeX, m_sizeY );
	glNamedFramebufferRenderbuffer( m_framebuffer, attachmentPoint, GL_RENDERBUFFER, m_colorAttachment );
}

void framebuffer::addDepthAttachment( const GLenum depthFormat ) {
	glCreateRenderbuffers( 1, &m_depthAttachment );
	if( GL_TRUE != glIsRenderbuffer( m_depthAttachment ) )
		logbook::log_msg( logbook::RENDERER, logbook::ERROR, "Error creating depth renderbuffer" );
	glNamedRenderbufferStorage( m_depthAttachment, depthFormat, m_sizeX, m_sizeY );
	glNamedFramebufferRenderbuffer( m_framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthAttachment );
	/*glTextureParameteri( m_depthAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTextureParameteri( m_depthAttachment, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );*/
}

bool framebuffer::isComplete() const {
	return GL_FRAMEBUFFER_COMPLETE != glCheckNamedFramebufferStatus( m_framebuffer, GL_FRAMEBUFFER ) ? false : true;
}

void framebuffer::bind( const GLenum target ) const {
	glBindFramebuffer( target, m_framebuffer );
}

void framebuffer::resize( const unsigned int x, const unsigned int y ) {
	m_sizeX = x;
	m_sizeY = y;
	if( GL_TRUE == glIsRenderbuffer( m_colorAttachment ) ) {
		//glInvalidateNamedframebufferData( m_framebuffer, 1, &m_colorAttachment );
		glDeleteRenderbuffers( 1, &m_colorAttachment );
		addColorAttachment( m_colorFormat );
	}
	if( GL_TRUE == glIsRenderbuffer( m_depthAttachment ) ) {
		//glInvalidateNamedframebufferData( m_framebuffer, 1, &m_depthAttachment );
		glDeleteRenderbuffers( 1, &m_depthAttachment );
		addDepthAttachment( m_depthFormat );
	}
}

void framebuffer::clear( const color_t &clearColor ) const {
	glClearColor( clearColor.x, clearColor.y, clearColor.z, clearColor.w );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

}
