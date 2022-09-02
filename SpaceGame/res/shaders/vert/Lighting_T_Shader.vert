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
uniform mat4 u_LightMVP;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;
out vec4 v_LightSpacePos;

void main()
{	
	gl_Position = u_Proj * u_View * u_Model * position;
	v_TexCoord = texCoord;
	v_Normal = mat3(u_InvTranspModel) * normal;

	//Light
	v_FragPos = vec3(u_Model*position);
	v_LightSpacePos = u_LightMVP * position;
}