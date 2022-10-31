
#include "heightmap.h"
#include "base/logbook.h"
#include "settings.h"
#include "renderer/sampler.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <stb/stb_image.h>

using namespace orf_n;

namespace terrain {

heightmap::heightmap( const std::string &filename, const bit_depth depth ) :
				m_filename(filename), m_bit_depth(depth) {

	uint16_t *values_16{nullptr};
	uint8_t *values_8{nullptr};
	//stbi_set_flip_vertically_on_load( true );
	int w, h, num_channels;
	const std::string texture_file = filename+".png";
	if( B16 == depth )
		// load the data, single channel 16
		values_16 = stbi_load_16( texture_file.c_str(), &w, &h, &num_channels, 1 );
	if( B8 == depth ) {
		// load the data, single channel 8
		values_8 = stbi_load( texture_file.c_str(), &w, &h, &num_channels, 1 );
	}
	if( nullptr == values_16 && nullptr == values_8 ) {
		std::string s = "Error loading heightmap image file '"+texture_file+"'.";
		logbook::log_msg( logbook::TERRAIN, logbook::ERROR, s );
		// @todo throw std::runtime_error( s );
	}
	if( num_channels != 1 )
		logbook::log_msg(
				logbook::TERRAIN, logbook::WARNING,	"Unknown heightmap format in '"+texture_file+"'. Not a monochrome image ?"
		);

	m_extent = omath::uvec2( static_cast<unsigned int>(w), static_cast<unsigned int>(h) );
	unsigned int numPixels{ m_extent.x * m_extent.y };
	// TODO Check.
	m_height_values = new uint16_t[numPixels];
	memcpy(
			m_height_values,
			B8==depth ? (void *)values_8 : (void *)values_16,
			numPixels * (B8==depth ? sizeof(uint8_t) : sizeof(uint16_t))
	);
	// There's only float data 0..1 from now on
	glCreateTextures( GL_TEXTURE_2D, 1, &m_texture );
	glBindTextureUnit( HEIGHTMAP_TEXTURE_UNIT, m_texture );
	const GLenum internal_format = settings::USE_HALF_FLOATS ? GL_R16F : GL_R32F;
	glTextureStorage2D( m_texture, 1, internal_format, m_extent.x, m_extent.y );
	glTextureSubImage2D(
			m_texture, 0,					// texture and mip level
			0, 0, m_extent.x, m_extent.y,	// offset and size
			GL_RED, GL_UNSIGNED_SHORT, m_height_values
	);
	// set the default sampler for the heightmap texture
	set_default_sampler( m_texture, LINEAR_CLAMP );
	// release mem
	if( nullptr != values_8 )
		stbi_image_free( values_8 );
	if( nullptr != values_16 )
		stbi_image_free( values_16 );
	// @todo: query texture size !
	float size_in_kb{
		float( sizeof(*this) + numPixels * sizeof(uint16_t) ) / 1024.0f
	};
	std::ifstream bbf( filename+".bb", std::ios::in );
	if( !bbf.is_open() ) {
		std::ostringstream s;
		s << "Error opening bounding box file '" << filename << ".bb'. Tile will not be rendered correctly.";
		logbook::log_msg( logbook::TERRAIN, logbook::WARNING, s.str() );
	}
	omath::vec3 min, max;
	bbf >> min.x >> min.y >> min.z >> max.x >> max.y >> max.z;
	bbf.close();
	m_raster_aabb.m_min = omath::vec3{ min.x, min.y * settings::HEIGHT_FACTOR, min.z };
	m_raster_aabb.m_max = omath::vec3{ max.x, max.y * settings::HEIGHT_FACTOR, max.z };
	std::ostringstream s;
	s << "Heightmap texture '" << texture_file<<"' loaded. Raster bounding box: " << m_raster_aabb << ".\n"<<
		"\tTexture unit " << HEIGHTMAP_TEXTURE_UNIT <<", " << m_extent.x <<'*'<< m_extent.y << ", " << num_channels <<
		" channel(s). Size in memory : " << size_in_kb << "kB.";
	logbook::log_msg( logbook::TERRAIN, logbook::INFO, s.str() );
}

const GLuint &heightmap::get_texture() const {
	return m_texture;
}

const omath::uvec2 &heightmap::get_extent() const {
	return m_extent;
}

float heightmap::get_height_at( const unsigned int x, const unsigned int y ) const {
	return (float)m_height_values[x + y * m_extent.x] * settings::HEIGHT_FACTOR;
}

omath::vec2 heightmap::get_min_max_height_area(
		const unsigned int x, const unsigned int z, const unsigned int w, const unsigned int h ) const {
	omath::vec2 values{ std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
	for( unsigned int i = x; i < x + w; ++i )
	    for( unsigned int j = z; j < z + h; ++j ) {
	    	float new_val = get_height_at( i, j );
	        if( new_val < values.x )
	            values.x = new_val;
	        if( new_val > values.y )
	        	values.y = new_val;
	    }
	return omath::vec2{ values };
}

const heightmap::bit_depth &heightmap::get_depth() const {
	return m_bit_depth;
}

const omath::aabb &heightmap::get_raster_aabb() const {
	return m_raster_aabb;
}

heightmap::~heightmap() {
	unbind();
	glDeleteTextures( 1, &m_texture );
	delete [] m_height_values;
	logbook::log_msg( logbook::TERRAIN, logbook::INFO,
			"Heightmap '" + m_filename + "' destroyed." );
}

void heightmap::bind() const {
	glBindTextureUnit( HEIGHTMAP_TEXTURE_UNIT, m_texture );
}

void heightmap::unbind() const {
	glBindTextureUnit( HEIGHTMAP_TEXTURE_UNIT, 0 );
}

}
