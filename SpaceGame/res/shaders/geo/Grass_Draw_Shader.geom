#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 36) out;

uniform mat4 u_LightMVP;

in DATA
{
	vec2 v_TexCoord;
	vec3 v_Normal;
	mat4 v_Proj;
} data_in[];

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;
out vec4 v_LightSpacePos;

void main()
{
	for (float y = 0; y < 12; y+=0.5)
	{
		for (int i = 0; i < 3; i++)
		{
			vec4 position = gl_in[i].gl_Position + vec4(0.0,y,0.0,0.0);
			gl_Position = data_in[i].v_Proj * position;
			v_Normal = data_in[i].v_Normal;
			v_TexCoord = data_in[i].v_TexCoord;
			v_LightSpacePos = u_LightMVP * position;
			v_FragPos = vec3(gl_Position);
			EmitVertex();
		}
		EndPrimitive();
	}
}