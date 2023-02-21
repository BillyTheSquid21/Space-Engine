#version 430 core

in vec3 v_Normal;
in vec2 v_Tex;
in vec3 v_ViewPos;
in vec3 v_FragPos;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec3 u_Ambient;
uniform vec3 u_Diffuse;
uniform vec3 u_Specular;
uniform float u_Shininess;

struct PointLight
{
    vec3 position;
    float brightness;
    vec3 color;
    float radius;
};

struct DirectionalLight
{
    vec3 direction;
    float brightness;
    vec3 color;
    float light_buffer;
};

layout(std430, binding = 1) buffer SG_Lighting
{
    vec3 ambient_color;
    float ambient;
    DirectionalLight dir_light;
    PointLight point_lights[];
};

void main()
{
    //Get Tex
    vec3 baseCol = vec3(texture(u_Texture, v_Tex));

    //Ambient
    vec3 ambientCol = ambient * ambient_color;

    //Diffuse
    vec3 diffuseCol = dir_light.brightness * max(dot(v_Normal, dir_light.direction), 0.0) * dir_light.color;

    //Specular
    float shininess = 1.0f * float(u_Shininess == 0.0f) + (u_Shininess * float(u_Shininess != 0.0f)); //Ensure non 0
    vec3 viewDir = normalize(v_ViewPos - v_FragPos);
    vec3 reflectDir = reflect(-dir_light.direction, v_Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specularCol =  dir_light.brightness * u_Specular * spec * dir_light.color;  

    //Result
    vec3 resultantCol = (ambientCol + diffuseCol + specularCol) * baseCol;

    FragColor = vec4(resultantCol, 1.0);
}