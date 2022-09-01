#version 330 core

in vec2 v_Position;

out vec4 FragColor;

uniform float u_Time;
uniform float u_Height;

void main()
{
	//Calculate division constants for bar
	float barConst = u_Height / ((-0.0165f*u_Height) + 48.5f);
	float thickConst = ((0.0567f*u_Height) - 29.43f);

	int thick = int(thickConst);
	int bar = int(barConst * sin(u_Time*1.5f));

	if (int(v_Position.y) % thick < bar)
	{
		FragColor = vec4(0.05,0.05,0.05,1.0);
		return;
	}
	FragColor = vec4(0.0);
	return;
}