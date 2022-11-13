#version 330

layout(location = 0) in vec3 vtxCoord;
layout(location = 1) in vec2 vtxUv;
layout(location = 2) in vec3 vtxN;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;

out vec3 light_vector;
out vec3 normal_vector;
out vec3 halfway_vector;

// out vec2 tex_coord;

void main() {
	gl_Position =P * V * M * vec4(vtxCoord, 1.0);
	vec4 v = V * M * vec4(vtxCoord, 1.0);
	vec3 normal1 = normalize(vtxN);

	light_vector = normalize((V * vec4(lightPos, 1.0)).xyz - v.xyz);
	normal_vector = (inverse(transpose(V * M)) * vec4(normal1, 0.0)).xyz;
    halfway_vector = light_vector + normalize(-v.xyz);

	// tex_coord = texture.xy;
}