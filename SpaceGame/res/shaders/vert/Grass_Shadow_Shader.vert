#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 offset;

out DATA
{
	vec3 v_Normal;
	vec4 v_Color;
} data_out;

void main()
{	
	gl_Position = position + vec4(offset,0.0);
	data_out.v_Normal = vec3(0.0,0.0,1.0);
	data_out.v_Color = color;
}