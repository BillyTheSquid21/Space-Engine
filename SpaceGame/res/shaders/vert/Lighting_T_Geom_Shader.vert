#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

//MVP
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

//Lighting
uniform mat4 u_InvTranspModel;

out DATA
{
	vec2 v_TexCoord;
	vec3 v_Normal;
	mat4 v_Proj;
} data_out;

void main()
{	
	gl_Position = u_Model * position;
	data_out.v_TexCoord = texCoord;
	data_out.v_Normal = mat3(u_InvTranspModel) * normal;
	data_out.v_Proj = u_Proj * u_View * u_Model;
}