#version 330 core

layout(location = 0) in vec4 position;

uniform mat4 u_View;
uniform mat4 u_Proj;

out vec2 v_Position;

void main()
{
	v_Position = vec2(position.x, position.y);
	gl_Position = u_Proj * u_View * mat4(1.0) * vec4(position);
}
