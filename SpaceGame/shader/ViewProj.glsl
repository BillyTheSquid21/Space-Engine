layout(std140) uniform SG_ViewProjection
{
    mat4 View;
    mat4 Proj;
    vec3 ViewPos;
    float Width;
    float Height;
    float NearPlane;
    float FarPlane;
    float LogFN;
};