#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

//MVP
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

//Lighting
uniform mat4 u_InvTranspModel;
uniform mat4 u_LightMVP;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;
out vec4 v_LightSpacePos;

void main()
{	
	gl_Position = u_Proj * u_View * u_Model * vec4(position);
	v_TexCoord = texCoord;
	v_Normal = mat3(u_InvTranspModel) * normal;

	//Light
	v_FragPos = vec3(u_Model*position);
	v_LightSpacePos = u_LightMVP * position;
}

#shader fragment
#version 330 core

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_FragPos;
in vec4 v_LightSpacePos;

layout(location = 0) out vec4 color;

uniform sampler2D u_Texture;
uniform vec3 u_AmbLight;
uniform vec3 u_LightDir;
uniform sampler2D u_ShadowMap;
uniform int u_LightsActive;

float calcShadowFactor()
{
	vec3 ProjCoords = v_LightSpacePos.xyz / v_LightSpacePos.w;
    vec2 UVCoords;
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;
    float z = 0.5 * ProjCoords.z + 0.5;
    float Depth = texture(u_ShadowMap, UVCoords).x;

	if (Depth < (z-0.003))
        return 1.0; //is in shadow
    else
        return 0.0;
}

void main()
{	
	vec4 texColor = texture(u_Texture, v_TexCoord);
	if (texColor.a == 0.0){
		discard;
	}

	//Amb
	vec3 ambientLight = u_AmbLight;

	//Diffuse
	float diff = max(dot(v_Normal, u_LightDir), 0.0);
	vec3 diffuse = diff * vec3(0.5,0.5,0.5);
	
	//Shadow
	float shadow = calcShadowFactor();

	//Total
	color = vec4(ambientLight+((1.0f-shadow)*diffuse),1.0) * texColor;
	if (u_LightsActive == 1)
	{
		color = texColor;
	}
}