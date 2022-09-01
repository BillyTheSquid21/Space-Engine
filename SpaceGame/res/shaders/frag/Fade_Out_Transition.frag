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