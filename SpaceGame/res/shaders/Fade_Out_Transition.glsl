#shader vertex
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

#shader fragment
#version 330 core

in vec2 v_Position;

out vec4 FragColor;

uniform float u_Time;
uniform float u_FadeCap;

void main()
{
	float fade = u_Time / u_FadeCap;
	if (fade >= 0.99)
	{
		fade = 1.0;
	}

	FragColor = vec4(0.0, 0.0, 0.0, 0.0 + fade);
	return;
}