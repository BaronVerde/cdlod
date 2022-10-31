
#include "settings.h"
#include "simple_heightmap.h"
#include "base/logbook.h"
#include "renderer/sampler.h"
#include <iostream>
#include <cfloat>
#include <stb/stb_image.h>
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace orf_n;

namespace terrain {

simple_heightmap::simple_heightmap( const std::string &filename ) : m_filename(filename) {
	std::filesystem::path pathf(filename+".png");
	std::filesystem::path pathb(filename+".bb");
	if( !std::filesystem::exists(pathf) || !std::filesystem::exists(pathb) ) {
		logbook::log_msg(
				logbook::TERRAIN, logbook::WARNING,
				"Heightmap file '"+filename+"' or '.bb' not found."
		);
		throw std::runtime_error("File not found");
	}
	uint16_t *heightValues{nullptr};
	//stbi_set_flip_vertically_on_load( true );
	int w, h, numChannels;
	// load the data, single channel 16
	heightValues = stbi_load_16( (m_filename+".png").c_str(), &w, &h, &numChannels, 1 );
	if( nullptr == heightValues ) {
		logbook::log_msg(
				logbook::TERRAIN, logbook::ERROR,
				"Error loading heightmap image file '"+filename+".png'. Will not be rendered."
		);
		throw std::runtime_error("Data error");
	} else if( numChannels != 1 || w != h || !omath::is_power_of_2(w) || w < 256 ) {
		logbook::log_msg(
				logbook::TERRAIN, logbook::WARNING,
				"Unknown heightmap format in '" + m_filename + "'. Is it not a monochrome image and "\
				"not power-of-2 and >=256 and not square ? Will not be rendered."
		);
		stbi_image_free( heightValues );
		throw std::runtime_error("Data error");
	}
	m_size = omath::uvec2{ (unsigned int)w, (unsigned int)h };
	unsigned int numPixels =  m_size.x * m_size.y;
	// Unclamped values are allways stored as 16 bit integers
	m_height_values_normalized = new float[numPixels];
	unsigned int index = 0;
	for(unsigned int z=0; z < m_size.y; ++z ) {
		for(unsigned int x=0; x < m_size.x; ++x ) {
			const float t = float(heightValues[x+m_size.x*z] / 65535.0f);
			m_height_values_normalized[index++] = t;
		}
	}
	// There's only float data 0..1 from now on
	glCreateTextures( GL_TEXTURE_2D, 1, &m_texture );
	// no mip levels
	glTextureStorage2D( m_texture, 1, GL_R32F, m_size.x, m_size.y );
	glTextureSubImage2D(
			m_texture, 0,			// texture and mip level
			0, 0, m_size.x, m_size.y,	// offset and size
			GL_RED, GL_FLOAT, m_height_values_normalized
	);
	glBindTextureUnit( settings::HEIGHTMAP_TEXTURE_UNIT, m_texture );
	// set the default sampler for the heightmap texture
	set_default_sampler( m_texture, LINEAR_CLAMP );
	// release mem
	if( nullptr != heightValues )
		stbi_image_free( heightValues );
	// TODO query texture size !
	float totalSizeInKB{
		float( sizeof(*this) + sizeof(omath::aabb) + numPixels * sizeof(float) ) / 1024.0f
	};
	// Bounding box
	std::ifstream bbf{ filename + ".bb", std::ios::in };
	if( !bbf.is_open() )
		orf_n::logbook::log_msg(
				orf_n::logbook::TERRAIN, orf_n::logbook::WARNING,
				"Error opening bounding box file '"+filename+".bb'. Tile will not be rendered correctly."
		);
	else {
		omath::vec3 min, max;
		bbf >> min.x >> min.y >> min.z >> max.x >> max.y >> max.z;
		bbf.close();
		m_raster_aabb.m_min = { min.x,min.y,min.z };
		m_raster_aabb.m_max = { max.x,max.y,max.z };
		m_world_aabb.m_min = {
				m_raster_aabb.m_min.x * settings::raster_to_world_x,
				m_raster_aabb.m_min.y,
				m_raster_aabb.m_min.z * settings::raster_to_world_z
		};
		m_world_aabb.m_max = {
				m_raster_aabb.m_max.x * settings::raster_to_world_x,
				m_raster_aabb.m_max.y,
				m_raster_aabb.m_max.z * settings::raster_to_world_z
		};
	}
	std::ostringstream s;
	s << "Heightmap '"<<m_filename<<"', texture unit "<<settings::HEIGHTMAP_TEXTURE_UNIT<<", "<<m_size.x<<
			'*'<<m_size.y<<", "<<numChannels<<" channel(s) loaded. Size in memory : "<<totalSizeInKB<<
			"kB. Bounding box: "<<m_raster_aabb<<'.';
	logbook::log_msg( logbook::TERRAIN, logbook::INFO,s.str() );
}

const omath::uvec2 &simple_heightmap::get_size() const {
	return m_size;
}

float simple_heightmap::get_height_at(const unsigned int x,const unsigned int z) const {
	return m_height_values_normalized[x + z * m_size.x] * 65535.0f;
}

omath::vec2 simple_heightmap::get_area_min_max_height(
		const unsigned int x,const unsigned int z,const unsigned int w,const unsigned int h ) const {
	omath::vec2 values{ FLT_MAX, 0.0f };
	for(unsigned int i = x; i < x + w; ++i ) {
	    for(unsigned int j = z; j < z + h; ++j ) {
	    	float newVal = get_height_at( i, j );
	        if( newVal < values.x )
	            values.x = newVal;
	        if( newVal > values.y )
	        	values.y = newVal;
	    }
	}
	return omath::vec2{ values };
}

simple_heightmap::~simple_heightmap() {
	glDeleteTextures( 1, &m_texture );
	if(m_height_values_normalized)
		delete [] m_height_values_normalized;
	logbook::log_msg(
			logbook::TERRAIN, logbook::INFO,"Heightmap '" + m_filename + "' destroyed."
	);
}

void simple_heightmap::bind() const {
	glBindTextureUnit( settings::HEIGHTMAP_TEXTURE_UNIT, m_texture );
}

const omath::aabb &simple_heightmap::get_raster_aabb() const {
	return m_raster_aabb;
}

const omath::aabb &simple_heightmap::get_world_aabb() const {
	return m_world_aabb;
}

}
