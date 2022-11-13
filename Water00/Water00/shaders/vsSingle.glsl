#version 330
layout(location = 0) in vec3 vtxCoord;
layout(location = 1) in vec2 vtxUv;
layout(location = 2) in vec3 vtxN;

out vec2 uv;
out vec3 worldPos;
out vec3 worldN;
out vec3 halfway_vector;
out vec3 light_vector;

uniform mat4 M, V, P;
uniform sampler2D texDisp;
uniform vec2 dudvMove;
uniform vec3 eyePoint;
uniform vec3 light_position;


void main() {
  uv = vtxUv;

  worldPos = (M * vec4(vtxCoord, 1.0)).xyz;
  float disAtten=max(length(eyePoint-worldPos),0.01);
  disAtten=exp(-0.05f * disAtten);

  vec3 scale=vec3(0.75,0.5,0.5);

  vec3 disp = texture(texDisp, uv).rgb;


  worldPos.x+=disp.x;

  worldPos.z+=disp.z;

  worldPos.y += disp.y;

  gl_Position=P*V*vec4(worldPos,1.0);

  worldPos = (M * vec4(vtxCoord, 1.0)).xyz;

  vec4 v=V*vec4(worldPos,1.0);

  halfway_vector=light_vector + normalize(-v.xyz);

  light_vector=normalize((V*vec4(light_position,1.0f)).xyz-v.xyz);

  worldN = (vec4(vtxN, 1.0) * inverse(M)).xyz;
  worldN = normalize(worldN);
}
