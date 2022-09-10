#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec4 offset;

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
	float v_HeightVariance;
} data_out;

float calcOffset()
{
	return sin(offset.w) + ((cos(offset.w/5.0))/2.0) + (sin(offset.w/10.0)/-4.0);
}

void main()
{	
	//Slightly offset color and height of blades
	float heightVar = calcOffset();
	data_out.v_HeightVariance = 2*heightVar;

	float colorOff = 10/sin(offset.w);
	data_out.v_Color = color + vec4(heightVar/colorOff, heightVar/colorOff, heightVar/colorOff,0.0);

	gl_Position = u_Model * (position + vec4(offset.xyz,0.0));
	data_out.v_Proj = u_Proj * u_View * u_Model;
	data_out.v_Normal = vec3(0.0,0.0,1.0);
}