#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 36) out;

uniform mat4 WVP;

in DATA
{
	vec2 v_TexCoord;
	vec3 v_Normal;
} data_in[];

out vec2 v_TexCoord;

void main()
{
	for (float y = 0; y < 12; y+=0.5)
	{
		for (int i = 0; i < 3; i++)
		{
			vec4 position = gl_in[i].gl_Position + vec4(0.0,y,0.0,0.0);
			gl_Position = WVP * position;
			v_TexCoord = data_in[i].v_TexCoord;
			EmitVertex();
		}
		EndPrimitive();
	}
}