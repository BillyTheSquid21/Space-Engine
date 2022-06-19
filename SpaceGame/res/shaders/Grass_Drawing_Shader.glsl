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

out DATA
{
	vec2 v_TexCoord;
	vec3 v_Normal;
	mat4 v_Proj;
} data_out;

void main()
{	
	gl_Position = u_Model * position;
	data_out.v_TexCoord = texCoord;
	data_out.v_Normal = mat3(u_InvTranspModel) * normal;
	data_out.v_Proj = u_Proj * u_View * u_Model;
}

#shader geometry
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

out vec2 g_TexCoord;
out vec3 g_Normal;
out vec3 g_FragPos;
out vec4 g_LightSpacePos;

void main()
{
	for (float y = 0; y < 12; y+=0.5)
	{
		for (int i = 0; i < 3; i++)
		{
			vec4 position = gl_in[i].gl_Position + vec4(0.0,y,0.0,0.0);
			gl_Position = data_in[i].v_Proj * position;
			g_Normal = data_in[i].v_Normal;
			g_TexCoord = data_in[i].v_TexCoord;
			g_LightSpacePos = u_LightMVP * position;
			g_FragPos = vec3(gl_Position);
			EmitVertex();
		}
		EndPrimitive();
	}
}

#shader fragment
#version 330 core

in vec2 g_TexCoord;
in vec3 g_Normal;
in vec3 g_FragPos;
in vec4 g_LightSpacePos;

layout(location = 0) out vec4 color;

uniform sampler2D u_Texture;
uniform vec3 u_AmbLight;
uniform vec3 u_LightDir;
uniform sampler2D u_ShadowMap;
uniform int u_LightsActive;

float calcShadowFactor()
{
	vec3 ProjCoords = g_LightSpacePos.xyz / g_LightSpacePos.w;
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
	vec4 texColor = texture(u_Texture, g_TexCoord);
	if (texColor.a == 0.0){
		discard;
	}

	//Amb
	vec3 ambientLight = u_AmbLight;

	//Diffuse
	float diff = max(dot(g_Normal, u_LightDir), 0.0);
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