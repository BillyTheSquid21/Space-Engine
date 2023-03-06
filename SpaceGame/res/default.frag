#version 430 core

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

struct LGridElement
{
    uint offset;
    uint size;
};

layout(std430, binding = 3) buffer SG_LightGrid
{
    LGridElement lightGrid[];
};

layout(std430, binding = 4) buffer SG_Tile_Light_Index
{
    int tileLightIndices[];
};

struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
};

layout(std140) uniform SG_ViewProjection
{
    mat4 View;
    mat4 Proj;
    vec3 ViewPos;
    float Width;
    float Height;
    float NearPlane;
    float FarPlane;
    float ViewPadding;
};

layout(std430, binding = 2) buffer SG_Cluster
{
    Cluster clusters[];
};

uint GetZSlice(float z)
{
    return uint((log(-z)*(48/(log(FarPlane/NearPlane))) - ((48*log(NearPlane))/log(FarPlane/NearPlane))));
}

uint getClusterIndex(float depth){
    uint clusterZVal  = GetZSlice(depth);

    uvec3 clusters    = uvec3(gl_FragCoord.x / 120.0, gl_FragCoord.y / 120.0, clusterZVal);
    
    uint clusterIndex = clusters.x +
                        16 * clusters.y +
                        (16 * 9) * clusters.z;
    return clusterIndex;
}

vec3 GetPointLightColor(vec3 norm)
{
    //Get index of Cluster
    uint cluster = getClusterIndex(v_ViewSpacePos.z);

    //Get light grid offset and size
    uint offset = lightGrid[cluster].offset;
    uint size = lightGrid[cluster].size;

    vec3 ptcol = vec3(0.0);
    for (uint i = offset; i < offset + size; i++)
    {
        int ptindex = tileLightIndices[i];
        
        //1. Get distance
        float ptdist = length(point_lights[ptindex].position - v_FragPos);
        float distFromRad = ptdist - point_lights[ptindex].radius;

        //2. Get direction and diff
        vec3 ptDir = normalize(point_lights[ptindex].position - v_FragPos);
        float ptDiff = max(dot(norm, ptDir), 0.0);

        //3. Change constants by product of light rad * brightness;
        float sizeFactor = point_lights[ptindex].radius * point_lights[ptindex].brightness;

        //4. Work out attenuation
        float adjustedLin = point_linear / sizeFactor;
        float adjustedQua = point_quad / sizeFactor;

        float ptatten = 1.0 / (point_constant + (adjustedLin * (distFromRad)) + (adjustedQua * distFromRad * distFromRad));
        
        if (ptatten > point_cutoff)
        {
            ptcol = ptcol + (ptDiff * ptatten * point_lights[ptindex].color * point_lights[ptindex].brightness);
        }
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

    //Very shakey test
    uint index = getClusterIndex(v_ViewSpacePos.z);
    uint size = lightGrid[index].size;
    Cluster cluster = clusters[index];

    if (u_ShowLights == 0)
    {
        FragColor = vec4(resultantCol, 1.0);
    }

    if (u_ShowLights == 1)
    {
        FragColor = vec4(float(size*size)/2048.0, float(size)/50.0, 0.0, 1.0);
        FragColor = FragColor + (vec4(resultantCol,1.0)/2.0);
    }
}