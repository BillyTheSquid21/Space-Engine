#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

out vec2 v_TexCoord;

void main()
{	
	gl_Position = u_Proj * u_View * u_Model * vec4(position);
	v_TexCoord = texCoord;
}

#shader fragment
#version 330 core

in vec2 v_TexCoord;

layout(location = 0) out vec4 color;

uniform sampler2D u_Texture;

void main()
{	
	vec4 texColor = texture(u_Texture, v_TexCoord);
	if (texColor.a == 0.0){
		discard;
	}
	color = texColor;
}