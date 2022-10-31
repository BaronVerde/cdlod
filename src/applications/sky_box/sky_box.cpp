
#include "renderer/uniform.h"
#include "scene/scene.h"
#include "sky_box.h"
#include "base/logbook.h"
#include "applications/camera/camera.h"
#include "omath/vec3.h"
#include "omath/mat4.h"
#include <sstream>
#include <stb/stb_image.h>

using namespace orf_n;

sky_box::sky_box( const std::vector<std::string> &files ) :
		renderable("sky_box") {
	if( 6 != files.size() ) {
		m_faces.clear();
		m_faces.push_back("resources/cubemaps/eso_pos_x.png");
		m_faces.push_back("resources/cubemaps/eso_neg_x.png");
		m_faces.push_back("resources/cubemaps/eso_pos_y.png");
		m_faces.push_back("resources/cubemaps/eso_neg_y.png");
		m_faces.push_back("resources/cubemaps/eso_pos_z.png");
		m_faces.push_back("resources/cubemaps/eso_neg_z.png");
		/*"resources/cubemaps/starfield_rt.tga",
			"resources/cubemaps/starfield_lf.tga",
			"resources/cubemaps/starfield_up.tga",
			"resources/cubemaps/starfield_dn.tga",
			"resources/cubemaps/starfield_ft.tga",
			"resources/cubemaps/starfield_bk.tga"*/
		/*"resources/cubemaps/nasa_pos_x.png",
			"resources/cubemaps/nasa_neg_x.png",
			"resources/cubemaps/nasa_pos_y.png",
			"resources/cubemaps/nasa_neg_y.png",
			"resources/cubemaps/nasa_pos_z.png",
			"resources/cubemaps/nasa_neg_z.png"*/
	} else {
		m_faces = files;
	}
}

void sky_box::create_texture() {
	glCreateTextures( GL_TEXTURE_CUBE_MAP, 1, &m_texture );
	// Check size of 1st face
	int width, height, numChannels;
	//stbi_set_flip_vertically_on_load( true );
	// Test the data and formats
	stbi_uc *data = stbi_load( m_faces[0].c_str(), &width, &height, &numChannels, 0 );
	if( NULL == data ) {
		logbook::log_msg(
				logbook::RENDERER,logbook::ERROR,"Skybox texture '" + m_faces[0] + "' failed to load."
		);
		throw std::runtime_error("Error loading sky box. Lee log.");
	} else
		stbi_image_free( data );
	GLuint internalFormat, format;
	switch( numChannels ) {
		case 1 : internalFormat = GL_R8; format = GL_RED; break;
		case 3 : internalFormat = GL_RGB8; format = GL_RGB; break;
		case 4 : internalFormat = GL_RGBA8; format = GL_RGBA; break;
		default :
			std::string s="Unsupported cube map texture format. '"+m_faces[0]+
				"' failed to load. Check number of channels (1/3/4).";
			logbook::log_msg( logbook::RENDERER, logbook::ERROR, s );
			throw std::runtime_error( s );
			break;
	}
	glTextureStorage2D( m_texture, 1, internalFormat, width, height );
	// One mip level only
	for( unsigned int face{ 0 }; face < 6; ++face ) {
		data = stbi_load( m_faces[face].c_str(), &width, &height, &numChannels, 0 );
		if( NULL != data ) {
			glTextureSubImage3D( m_texture, 0, 0, 0,// target, level , xOffset, yOffset
					face, width, height, 1, format,		// cube map face, width, height, 1 face a time, format
					GL_UNSIGNED_BYTE, data );			// datatype, data pointer
			stbi_image_free( data );
			data = NULL;
		} else {
			std::string s="Cubemap texture '" + m_faces[face] + "' failed to load.";
			logbook::log_msg( logbook::RENDERER, logbook::WARNING, s );
		}
	}
	glTextureParameteri( m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTextureParameteri( m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTextureParameteri( m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTextureParameteri( m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTextureParameteri( m_texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	std::ostringstream s;
	s << "Cubemap '" << m_faces[0] << "' (" << width << '*' << height <<
			") " << numChannels << " channels and following 5 files loaded.";
	logbook::log_msg( logbook::RENDERER, logbook::INFO, s.str() );
}

void sky_box::setup() {
	create_texture();
	// Create the shaders
	std::vector<std::shared_ptr<module>> modules;
	modules.push_back(
			std::make_shared<module>( GL_VERTEX_SHADER,"src/applications/sky_box/sky_box.vert.glsl" )
	);
	modules.push_back(
			std::make_shared<module>( GL_FRAGMENT_SHADER,"src/applications/sky_box/sky_box.frag.glsl" )
	);
	m_program = std::make_unique<program>( modules );
	/*m_program->use();
	omath::mat4 model_m = omath::rotate( omath::mat4(1.0f), omath::radians(60.0f), omath::vec3(0.0f,0.0f,1.0f) );
	set_uniform( m_program->get_program(), "modelM", model_m );*/
	std::vector<omath::vec3> vertices{
		// positions
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f, -1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{  1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{ -1.0f, -1.0f, -1.0f },
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f,  1.0f,  1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{  1.0f, -1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f, -1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{ -1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f, -1.0f,  1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{  1.0f,  1.0f, -1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{  1.0f,  1.0f,  1.0f },
		omath::vec3{ -1.0f,  1.0f,  1.0f },
		omath::vec3{ -1.0f,  1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{  1.0f, -1.0f, -1.0f },
		omath::vec3{ -1.0f, -1.0f,  1.0f },
		omath::vec3{  1.0f, -1.0f,  1.0f }
	};
	glCreateVertexArrays( 1, &m_vao );
	glCreateBuffers( 1, &m_vertex_buffer );
	glNamedBufferData( m_vertex_buffer,vertices.size()*sizeof(omath::vec3),vertices.data(),GL_STATIC_DRAW );
	// offset into buffer is 0 and stride is sizeof( vec3 )
	glVertexArrayVertexBuffer(m_vao, SKYBOX_VERTEX_BUFFER_BINDING_INDEX, m_vertex_buffer, 0, sizeof(omath::vec3) );
	glVertexArrayAttribBinding( m_vao, 0, SKYBOX_VERTEX_BUFFER_BINDING_INDEX );
	glVertexArrayAttribFormat( m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0 );
	glEnableVertexArrayAttrib( m_vao, 0);
	logbook::log_msg( logbook::RENDERER, logbook::INFO, "Skybox created." );
}

void sky_box::render(const double delta_time) {
	// draw sky_box at last !
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	// TODO try glBindTextureUnit and use a sampler ?
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER,m_vertex_buffer);
    glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
	m_program->use();
	set_uniform(
			m_program->get_program(), "projectionView", omath::mat4(m_scene->get_camera()->get_untranslated_view_perspective_matrix())
	);
	glDrawArrays( GL_TRIANGLES, 0, 36 );
	glDepthFunc( GL_LESS );
}

void sky_box::cleanup() {}

sky_box::~sky_box() {
	glDeleteVertexArrays( 1, &m_vao );
	if(glIsTexture(m_texture))
		glDeleteTextures(1, &m_texture);
	logbook::log_msg(
			logbook::RENDERER, logbook::INFO,"Skybox texture #" + std::to_string(m_texture) + " destroyed."
	);
}
