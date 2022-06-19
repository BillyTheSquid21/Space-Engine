#shader vertex
#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

out vec2 v_TexCoord;

out DATA
{
	vec2 v_TexCoord;
} data_out;

void main()
{
	data_out.v_TexCoord = texCoord;
	gl_Position = position;
}

#shader geometry
#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 36) out;

uniform mat4 WVP;

in DATA
{
	vec2 v_TexCoord;
} data_in[];

out vec2 g_TexCoord;

void main()
{
	for (float y = 0; y < 12; y+=0.5)
	{
		for (int i = 0; i < 3; i++)
		{
			vec4 position = gl_in[i].gl_Position + vec4(0.0,y,0.0,0.0);
			gl_Position = WVP * position;
			g_TexCoord = data_in[i].v_TexCoord;
			EmitVertex();
		}
		EndPrimitive();
	}
}

#shader fragment
#version 330

uniform sampler2D u_Texture;

in vec2 g_TexCoord;

void main()
{
	float alpha = texture(u_Texture, g_TexCoord).a;
	if (alpha == 0)
	{
		discard;
	}
	gl_FragDepth = gl_FragCoord.z;
}  