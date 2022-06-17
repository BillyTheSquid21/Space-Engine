#shader vertex
#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 WVP;

out vec2 v_TexCoord;

void main()
{
	vec4 adjustedPos = WVP*position;
	v_TexCoord = texCoord;
	gl_Position = adjustedPos;
}

#shader fragment
#version 330

uniform sampler2D u_Texture;

in vec2 v_TexCoord;

void main()
{
	float alpha = texture(u_Texture, v_TexCoord).a;
	if (alpha == 0)
	{
		discard;
	}
	gl_FragDepth = gl_FragCoord.z;
}  