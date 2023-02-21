#version 430 core

in vec3 v_Normal;
in vec2 v_Tex;
in vec3 v_ViewPos;
in vec3 v_FragPos;
in mat3 v_TBN;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform sampler2D u_NormalMap;

uniform vec3 u_Ambient;
uniform vec3 u_Diffuse;
uniform vec3 u_Specular;
uniform float u_Shininess;
uniform float u_Time;

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

    float point_constant;
    float point_linear;
    float point_quad;
    float point_cutoff;

    DirectionalLight dir_light;
    PointLight point_lights[];
};

vec3 GetPointLightColor(vec3 norm)
{
    //Point lights
    vec3 ptcol = vec3(0.0);

    for (int i = 0; i < point_lights.length(); i++)
    {
        vec3 testPos = point_lights[i].position;
        float testRadius = length(vec2(testPos.x, testPos.z) - vec2(0.0));
        testPos.x = testRadius * sin(u_Time + testPos.x);
        testPos.z = testRadius * cos(u_Time + testPos.z);
            
        //1. Get distance
        float ptdist = length(testPos - v_FragPos);
        float distFromRad = ptdist - point_lights[i].radius;

        //2. Get direction and diff
        vec3 ptDir = normalize(testPos - v_FragPos);
        float ptDiff = max(dot(norm, ptDir), 0.0);

        //3. Change constants by product of light rad * brightness;
        float sizeFactor = point_lights[i].radius * point_lights[i].brightness;

        //4. Work out attenuation
        float adjustedLin = point_linear / sizeFactor;
        float adjustedQua = point_quad / sizeFactor;

        float ptatten = 1.0 / (point_constant + (adjustedLin * (distFromRad)) + (adjustedQua * distFromRad * distFromRad));
        ptatten = (ptatten * float(distFromRad > 0.0)) + (1.0 * float(distFromRad <= 0.0));
    
        ptcol = ptcol + (ptDiff * ptatten * point_lights[i].color * point_lights[i].brightness);
    }
    return clamp(ptcol, 0.0, 1.0);
}

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

    vec3 ptcol = GetPointLightColor(normal);

    //Specular
    float shininess = 1.0f * float(u_Shininess == 0.0f) + (u_Shininess * float(u_Shininess != 0.0f)); //Ensure non 0
    vec3 viewDir = normalize(v_ViewPos - v_FragPos);
    vec3 reflectDir = reflect(-dir_light.direction, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specularCol =  dir_light.brightness * u_Specular * spec * dir_light.color;  

    //Result
    vec3 resultantCol = (ambientCol + diffuseCol + specularCol + ptcol) * baseCol;

    FragColor = vec4(resultantCol, 1.0);
}