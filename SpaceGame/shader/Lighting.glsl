#SGInclude shader/ViewProj.glsl

//Lighting structs, aligned to match C++ implementation
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

//CLUSTER
struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
};

layout(std430, binding = 2) buffer SG_Cluster
{
    int cluster_width;
    int cluster_height;
    int cluster_depth;
    float cluster_xpix;
    float cluster_ypix;
    float cluster_B1;
    float cluster_B2;
    float cluster_B3;
    Cluster cluster_clusters[];
};

//LIGHT GRID
struct LGridElement
{
    uint offset;
    uint size;
};

layout(std430, binding = 3) buffer SG_LightGrid
{
    LGridElement light_grid[];
};

//TILE LIGHTS
layout(std430, binding = 4) buffer SG_Tile_Light_Index
{
    int tile_light_indices[];
};

//Functions that calculate lighting

//Gets point light color
vec3 GetPointLightColor(vec3 norm, vec3 fragPos, float depth);

//Visualize the light levels
vec4 GetVisualizedLight(vec3 col, float depth);

//Get the Z aspect of the frustum
uint GetZSlice(float z)
{
    return uint((log(-z)*(cluster_depth/(LogFN)) - ((cluster_depth*log(NearPlane))/LogFN)));
}

//Gets the index within the cluster array from SS coords and depth
uint GetClusterIndex(float depth){
    uint clusterZVal  = GetZSlice(depth);

    uvec3 cluster    = uvec3(gl_FragCoord.x / cluster_xpix, gl_FragCoord.y / cluster_ypix, clusterZVal);
    
    uint clusterIndex = cluster.x +
                        cluster_width * cluster.y +
                        (cluster_width * cluster_height) * cluster.z;
    return clusterIndex;
}

vec3 GetPointLightColor(vec3 norm, vec3 fragPos, float depth)
{
    //Get index of Cluster
    uint cluster = GetClusterIndex(depth);

    //Get light grid offset and size
    uint offset = light_grid[cluster].offset;
    uint size = light_grid[cluster].size;

    vec3 ptcol = vec3(0.0);
    for (uint i = offset; i < offset + size; i++)
    {
        int ptindex = tile_light_indices[i];
        
        //1. Get distance
        float ptdist = length(point_lights[ptindex].position - fragPos);
        float distFromRad = ptdist - point_lights[ptindex].radius;

        //2. Get direction and diff
        vec3 ptDir = normalize(point_lights[ptindex].position - fragPos);
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

vec4 GetVisualizedLight(vec3 col, float depth)
{
    //Use size to get amount of lights in a tile 
    uint index = GetClusterIndex(depth);
    uint size = light_grid[index].size;
    Cluster cluster = cluster_clusters[index];

    //Work out color
    vec4 frag_color = vec4(float(size*size)/(point_lights.length()*point_lights.length()*0.5), float(size)/(point_lights.length()), 0.0, 1.0);
    frag_color = frag_color + (vec4(col,1.0)/2.0);

    return frag_color;
}