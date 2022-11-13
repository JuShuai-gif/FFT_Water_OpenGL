#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 position;
in vec3 normal;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 viewPos;


void main()
{    
    //ambient
    vec3 ambient=texture(texture_diffuse1, TexCoords).xyz * 0.05f;

    //diffuse
    vec3 lightDir=normalize(lightPos-position);
    vec3 n=normalize(normal);
    float diff=max(dot(lightDir,n),0.0);
    vec3 diffuse = diff * texture(texture_diffuse1, TexCoords).xyz;

    //spec
    vec3 viewDir=normalize(viewPos-position);
    vec3 reflectDir=reflect(-lightDir,n);
    vec3 halfDir=normalize(lightDir+viewDir);
    float spec=pow(max(dot(n,halfDir),0.0),64.0);
    vec3 specular = vec3(1.0,1.0,1.0) * spec;

    vec3 result=ambient+diffuse+specular;
    FragColor = vec4(result,1.0f);
    //FragColor = vec4(1.0,0.0,0.0,1.0);
}
