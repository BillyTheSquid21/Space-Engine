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

float calcBias()
{
	float bias = 0.0027*tan(acos(dot(v_Normal, u_LightDir)));
	bias = clamp(bias, 0, 0.01);
	return bias;
}

float calcShadowFactor()
{
	vec3 ProjCoords = v_LightSpacePos.xyz / v_LightSpacePos.w;
    ProjCoords.x = 0.5 * ProjCoords.x + 0.5;
	ProjCoords.y = 0.5 * ProjCoords.y + 0.5;

    float z = 0.5 * ProjCoords.z + 0.5;
    float Depth = texture(u_ShadowMap, ProjCoords.xy).x;

	if (Depth < (z-calcBias()))
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