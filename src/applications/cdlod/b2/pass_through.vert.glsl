
#version 450 core

#pragma debug(on)

layout( location = 0 ) in vec3 position;

layout (location = 7) uniform mat4 modelMatrix;
layout (location = 15) uniform mat4 projViewMatrix;
// This is just an offset, e.g. for wireframe stuff.
uniform float y_offset=0.0f;

void main() {
	vec3 pos = position;
	pos.y += y_offset;
	gl_Position = projViewMatrix * modelMatrix * vec4( pos, 1.0f );
}
