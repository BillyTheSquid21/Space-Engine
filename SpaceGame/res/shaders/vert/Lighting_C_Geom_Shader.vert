#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 offset;

//MVP
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

//Lighting
uniform mat4 u_InvTranspModel;

out DATA
{
	vec3 v_Normal;
	vec4 v_Color;
	mat4 v_Proj;
} data_out;

void main()
{	
	gl_Position = u_Model * (position + vec4(offset,0.0));
	data_out.v_Proj = u_Proj * u_View * u_Model;
	data_out.v_Normal = vec3(0.0,0.0,1.0);
	data_out.v_Color = color;
}