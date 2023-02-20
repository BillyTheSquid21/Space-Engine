#version 330 core
layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 norm;

layout(std140) uniform SG_ViewProjection
{
    mat4 Projection;
};

out vec3 v_Normal;
out vec2 v_Tex;

void main()
{
    vec4 position = vec4(pos.xyz, 1.0);
    gl_Position = Projection * position;

    v_Normal = norm;
    v_Tex = uv;
}