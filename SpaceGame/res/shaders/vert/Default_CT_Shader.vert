#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

out vec4 v_Color;
out vec2 v_TexCoord;

void main()
{	
	gl_Position = u_Proj * u_View * u_Model * vec4(position);
	v_Color = color;
	v_TexCoord = texCoord;
}