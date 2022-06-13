#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

uniform mat4 u_InvTranspModel;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;

void main()
{	
	gl_Position = u_Proj * u_View * u_Model * vec4(position);
	v_TexCoord = texCoord;
	v_Normal = mat3(u_InvTranspModel) * normal;
	v_FragPos = vec3(u_Model*position);
}

#shader fragment
#version 330 core

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_FragPos;

layout(location = 0) out vec4 color;

uniform sampler2D u_Texture;
uniform float u_AmbLight;
uniform vec3 u_LightDir;
uniform int u_LightScene;

void main()
{	
	vec4 texColor = texture(u_Texture, v_TexCoord);
	if (texColor.a == 0.0){
		discard;
	}

	//Amb
	vec3 ambientLight = vec3(u_AmbLight, u_AmbLight, u_AmbLight);

	//Diffuse
	float diff = max(dot(v_Normal, u_LightDir), 0.0);
	vec3 diffuse = diff * vec3(0.5,0.5,0.5);
	
	//Total
	color = vec4(ambientLight+diffuse,1.0) * texColor;
	if (u_LightScene == 0)
	{
		color = texColor;
	}
}