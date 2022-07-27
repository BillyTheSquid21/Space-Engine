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
uniform float u_Height;

void main()
{
	int thick = int(u_Height / 34.0f);
	int bar = int(16.0f * 2.2f * sin(u_Time*1.5f));

	if (int(v_Position.y) % thick < bar)
	{
		FragColor = vec4(0.0,0.0,0.0,1.0);
		return;
	}
	FragColor = vec4(0.0);
	return;
}