#version 330 core
#SGInclude shader/ViewProj.glsl

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 norm;
layout (location = 3) in vec3 tangent;

out vec4 v_Position;
out vec3 v_Normal;
out vec2 v_Tex;
out vec3 v_ViewPos;
out vec3 v_FragPos;
out vec3 v_ViewSpacePos;
out mat3 v_TBN;

void main()
{
    vec4 position = vec4(pos.xyz, 1.0);
    gl_Position = Proj * View * position;
    v_Position = gl_Position;

    v_Normal = norm;
    v_Tex = uv;
    v_ViewPos = ViewPos;
    v_FragPos = vec3(pos);
    v_ViewSpacePos = vec3(View * position);

    //TBN
    vec3 T = normalize(tangent);
    vec3 N = norm;
    vec3 B = normalize(cross(N, T));
    v_TBN = mat3(T, B, N);
}