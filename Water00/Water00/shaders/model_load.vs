#version 330

layout(location = 0) in vec3 vtxCoord;
layout(location = 1) in vec2 vtxUv;
layout(location = 2) in vec3 vtxN;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main() {
	gl_Position =P * V * M * vec4(vtxCoord, 1.0);
	vec3 normal1 = normalize(vtxN);
}