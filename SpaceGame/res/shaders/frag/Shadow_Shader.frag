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