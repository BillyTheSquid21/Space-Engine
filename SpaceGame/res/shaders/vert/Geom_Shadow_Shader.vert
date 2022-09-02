#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

out vec2 v_TexCoord;

out DATA
{
	vec2 v_TexCoord;
	vec3 v_Normal;
} data_out;

void main()
{
	data_out.v_TexCoord = texCoord;
	data_out.v_Normal = normal;
	gl_Position = position;
}