
/* Originally by Strugar's "CDLOD", edit for OpenGL and adapted to own framework,
 * TODO enhanced with double precision RTE from Ring/Cozzy "Virtual Globes" (dsfun90) 
 * and mapping of world positions to an ellipsoid. */ 

#version 450 core

layout( location = 0 ) in vec3 position;

layout( binding = 0 ) uniform sampler2D g_tileHeightmap;

uniform float u_height_factor = 1.0f;
uniform vec2 u_raster_to_world = vec2(1.0f,1.0f);

// Use linear filter manually. Not necessary if heightmap sampler is GL_LINEAR
// Uniform bool u_useLinearFilter = false;
// Lower left world cartesian coordinate of heightmap tile
uniform vec3 g_tileOffset;
// Size x/y/z of heightmap tile in number of posts and max height.
uniform vec3 g_tileScale;
// Max of terrain tile. Used to clamp triangles outside of horizontal texture range.
uniform vec2 g_tileMax;
// (width-1)/width, (height-1)/height. Width and height are the same.
uniform vec2 g_tileToTexture;
// width, height, 1/width, 1/height in number of posts TODO .xy is textureSize(sampler,0)
uniform vec4 g_heightmapTextureInfo;

// --- Node specific data. Set in the render loop for every npde ---
// TODO: these could be constants if all tiles are the same !
// .x = gridDim, .y = gridDimHalf, .z = oneOverGridDimHalf
uniform vec3 g_gridDim;
// x and z hold horizontal minimums, .y holds the y center of the bounding box
uniform vec3 g_nodeOffset;
// x and z hold the horizontal scale of the bb in world size, .w holds the current lod level
uniform vec4 g_nodeScale;
// distances for current lod level for begin and end of morphing
// TODO: These are static in the application for now. Make them dynamic.
uniform vec4 g_morphConsts;
uniform vec3 g_diffuseLightDir;
layout( location = 5 ) uniform vec3 u_camera_position;
layout( location = 15 ) uniform mat4 u_viewProjectionMatrix;

out VERTEX_OUTPUT {
	vec4 position;
	vec2 heightmapUV;
	vec3 lightDir;	// .xyz
	vec4 eyeDir;	// .xyz = eyeDir, .w = eyeDist
	float lightFactor;
	vec3 normal;
	float morphLerpK;
} vertOut;

// Returns position relative to current tile fur texture lookup. Y value unsued.
vec3 getTileVertexPos( vec3 inPosition ) {
	vec3 returnValue = inPosition * g_nodeScale.xyz + g_nodeOffset;
	returnValue.xz = min( returnValue.xz, g_tileMax );
	return returnValue;
}

// Calculate texture coordinates for the heightmap. Observe lod node's offset and scale.
vec2 calculateUV( vec2 vertex ) {
	vec2 heightmapUV = ( vertex.xy - g_tileOffset.xz ) / g_tileScale.xz;
	heightmapUV *= g_tileToTexture;
	heightmapUV += g_heightmapTextureInfo.zw * 0.5f;
	return heightmapUV;
}

// morphs vertex .xy from high to low detailed mesh position
vec2 morphVertex( vec3 inPosition, vec2 vertex, float morphLerpValue ) {
	vec2 decimals = ( fract( inPosition.xz * vec2( g_gridDim.y, g_gridDim.y ) ) * 
					vec2( g_gridDim.z, g_gridDim.z ) ) * g_nodeScale.xz;
	return vertex - decimals * morphLerpValue;
}

// Assumes linear filtering being enabled in sampler.
// TODO 8 bit not yet supported !
float sampleHeightmap( vec2 uv ) {
	return texture( g_tileHeightmap, uv ).r * 65535.0f * u_height_factor;
}

vec3 calculateNormal( vec2 uv ) {
	vec2 texel_size = g_heightmapTextureInfo.zw;
	// Assumes sampler is clamped!
	float n = sampleHeightmap( uv + vec2( 0.0f, -texel_size.x ) );
	float s = sampleHeightmap( uv + vec2( 0.0f, texel_size.x ) );
	float e = sampleHeightmap( uv + vec2( -texel_size.y, 0.0f ) );
	float w = sampleHeightmap( uv + vec2( texel_size.y, 0.0f ) );
	// Classic method. Low eps makes harder shadows
	float eps = 0.5f;	
	return normalize( vec3((w - e)/(2*eps), (n - s)/(2*eps), 1.0f) );
	/*
	// Central difference. Very crispy.
	vec3 sn = vec3( 0.0f , s - n, -( texel_size.y * 2.0f ) );
	vec3 ew = vec3( -( texel_size.x * 2.0f ), e - w, 0.0f );
	sn *= ( texel_size.y * 2.0f );
    ew *= ( texel_size.x * 2.0f );
    sn = normalize( sn );
    ew = normalize( ew );
    return normalize( cross( sn, ew ) );
    // TODO Sobel filter for finer control ?
	*/
}

void main() {
	// calculate position on the heightmap for height value lookup
	vec3 vertex = getTileVertexPos( position );

	// Pre-sample height to be able to precisely calculate morphing value.
	vec2 preUV = calculateUV( vertex.xz );
	vertex.y = sampleHeightmap( preUV );
	float eyeDistance = distance( vertex, u_camera_position );

	vertOut.morphLerpK = 1.0f - clamp( g_morphConsts.z - eyeDistance * g_morphConsts.w, 0.0f, 1.0f );
	vertex.xz = morphVertex( position, vertex.xz, vertOut.morphLerpK );

	vertOut.heightmapUV = calculateUV( vertex.xz );
	vertex.y = sampleHeightmap( vertOut.heightmapUV );

	// calculate world position in a linear, flat world
	vec3 world_position = vertex * vec3(u_raster_to_world.x,1.0f,u_raster_to_world.y);
	vertOut.position = u_viewProjectionMatrix * vec4( world_position, 1.0f );
	vec3 normal = calculateNormal( vertOut.heightmapUV );
	vertOut.normal = normalize( normal * g_tileScale.xyz );
	vertOut.lightDir = g_diffuseLightDir;
	vertOut.eyeDir = vec4( vertOut.position.xyz - u_camera_position, eyeDistance );
	vertOut.lightFactor = clamp( dot( normal, g_diffuseLightDir ), 0.0f, 1.0f );
	gl_Position = vertOut.position;
}

/*vec3 calculateNormal( vec2 uv, bool smooth_normal ) {
	vec2 texel_size = g_heightmapTextureInfo.zw;
	float n = sampleHeightmap( uv + vec2( 0.0f, -texel_size.x ) );
	float s = sampleHeightmap( uv + vec2( 0.0f, texel_size.x ) );
	float e = sampleHeightmap( uv + vec2( -texel_size.y, 0.0f ) );
	float w = sampleHeightmap( uv + vec2( texel_size.y, 0.0f ) );
	vec3 calc_norm;
	if(smooth_normal) {
		float eps = 1.0f;
		calc_norm = vec3((w - e)/(2*eps), (n - s)/(2*eps), 1.0f);
	} else {
		// Crispy central difference shadows
		vec3 sn = vec3( 0.0f , s - n, -( texel_size.y * 2.0f ) );
		vec3 ew = vec3( -( texel_size.x * 2.0f ), e - w, 0.0f );
		sn *= ( texel_size.y * 2.0f );
	    ew *= ( texel_size.x * 2.0f );
    	sn = normalize( sn );
	    ew = normalize( ew );
	 	calc_norm = cross(sn, ew);
	}
	return normalize(result);
}*/
