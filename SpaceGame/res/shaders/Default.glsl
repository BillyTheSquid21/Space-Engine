#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

out vec4 vColor;

void main()
{	
	gl_Position = u_Proj * u_View * u_Model * vec4(position);
	vColor = color;
}

#shader fragment
#version 330 core

in vec4 vColor;

out vec4 color;

void main()
{
	color = vColor;
}