#version 430 core
#SGInclude shader/Lighting.glsl

in vec4 v_Position;
in vec3 v_Normal;
in vec2 v_Tex;
in vec3 v_ViewPos;
in vec3 v_FragPos;
in vec3 v_ViewSpacePos;
in mat3 v_TBN;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform sampler2D u_NormalMap;

uniform vec3 u_Ambient;
uniform vec3 u_Diffuse;
uniform vec3 u_Specular;
uniform float u_Shininess;
uniform int u_ShowLights;

void main()
{
    //Get normal
    vec3 normal = v_TBN * (vec3(texture(u_NormalMap, v_Tex)) * 2.0 - 1.0);

    //Get Tex
    vec3 baseCol = vec3(texture(u_Texture, v_Tex));

    //Ambient
    vec3 ambientCol = ambient * (ambient_color + u_Ambient);

    //Diffuse
    vec3 diffuseCol = dir_light.brightness * max(dot(normal, dir_light.direction), 0.0) * (dir_light.color + u_Diffuse);

    vec3 ptcol = GetPointLightColor(normal, v_FragPos, v_ViewSpacePos.z);

    //Specular
    float shininess = 1.0f * float(u_Shininess == 0.0f) + (u_Shininess * float(u_Shininess != 0.0f)); //Ensure non 0
    vec3 viewDir = normalize(v_ViewPos - v_FragPos);
    vec3 reflectDir = reflect(-dir_light.direction, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specularCol =  dir_light.brightness * u_Specular * spec * dir_light.color;  

    //Result
    vec3 resultantCol = (ambientCol + diffuseCol + specularCol + ptcol) * baseCol;

    //Very shakey test
    if (u_ShowLights == 0)
    {
        FragColor = vec4(resultantCol, 1.0);
    }

    if (u_ShowLights == 1)
    {
        FragColor = GetVisualizedLight(resultantCol, v_ViewSpacePos.z);
    }
}