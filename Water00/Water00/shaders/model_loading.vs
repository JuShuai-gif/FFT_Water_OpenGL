#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 normal;
out vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

uniform vec4 WaveA;
uniform vec4 WaveB;
uniform vec4 WaveC;
uniform vec4 WaveD;

vec3 GerstnerWave(vec4 wave,vec3 p,inout vec3 tangent,inout vec3 binormal)
{
    float steepness = wave.z;
    float wavelength=wave.w;
    float k=2.0f*3.1415926f/wavelength;
    float c=sqrt(9.8f/k);
    vec2 d=normalize(wave.xy);
    float f=k*(dot(d,p.xz)-c*time);
    float a=steepness/k;

    tangent+=vec3(-d.x*d.x*(steepness*sin(f)),d.x*(steepness*cos(f)),-d.x*d.y*(steepness*sin(f)));
    binormal+=vec3(-d.x*d.y*(steepness*sin(f)),d.y*(steepness*cos(f)),-d.y*d.y*(steepness*sin(f)));

    return vec3(d.x*(a*cos(f)),a*sin(f),d.y*(a*cos(f)));
}

void main()
{
    //vec2 Direction = vec2(1.0f,1.0f);
    //vec2 d = normalize(Direction);

    //float f = k * (dot(d,pos.xz) - speed * time);
    //float a = speedness / k;

    //pos.x += d.x *(a * cos(f));
    //pos.y = a * sin(f);
    //pos.z += d.y * (a * cos(f));

    //vec3 tangent=normalize(vec3(1 - d.x * d.x * (speedness * sin(f)),d.x * (speedness * cos(f)),-d.x * d.y * (speedness * sin(f))));
    //vec3 binormal = vec3(-d.x * d.y * (speedness * sin(f)),d.y * (speedness * cos(f)),1 - d.y * d.y * (speedness * sin(f)));

    //normal = normalize(cross(binormal,tangent));

    //mat3 normalMatrix = transpose(inverse(mat3(model)));
    //normal = normalize(normalMatrix * aNormal);

    TexCoords = aTexCoords;
    vec3 pos = aPos;
    vec3 tangent=vec3(1.0f,0.0f,0.0f);
    vec3 binormal=vec3(0.0f,0.0f,1.0f);
    vec3 p=pos;
    p+=GerstnerWave(WaveA,pos,tangent,binormal);
    p+=GerstnerWave(WaveB,pos,tangent,binormal);
    p+=GerstnerWave(WaveC,pos,tangent,binormal);
    p+=GerstnerWave(WaveD,pos,tangent,binormal);
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));

    normal=normalize(normalMatrix * cross(binormal,tangent));

    position = (model * vec4(p,1.0)).xyz;
    gl_Position = projection * view * model * vec4(p, 1.0);

}