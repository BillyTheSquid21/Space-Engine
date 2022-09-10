#version 330 core

in vec3 v_Normal;
in vec3 v_FragPos;
in vec4 v_LightSpacePos;
in vec4 v_Color;

layout(location = 0) out vec4 color;

uniform sampler2D u_Texture;
uniform vec3 u_AmbLight;
uniform vec3 u_LightDir;
uniform sampler2D u_ShadowMap;
uniform int u_LightsActive;

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

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

	float shadowFac = 0.0;
	float samples = 4.0;
	float coeff = 1.0/samples;
	for (int i=0;i<samples;i++)
	{
		shadowFac += coeff*float((texture(u_ShadowMap, ProjCoords.xy + poissonDisk[i]/2000.0).x < (z-calcBias())));
	}
	return shadowFac;
}

void main()
{	
	vec4 texColor = v_Color;
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

	if (u_LightsActive == 1)
	{
		shadow = 0.0;
	}

	//Total
	color = vec4(ambientLight+((1.0f-shadow)*diffuse),1.0) * texColor;
}