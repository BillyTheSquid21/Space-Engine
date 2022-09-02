#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 u_LightMVP;
uniform sampler2D u_WindA;
uniform sampler2D u_WindB;
uniform float u_WeightA;

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

vec4 slerp(vec4 qa, vec4 qb, float t)
{
    float a = 1 - t;
    float b = t;
    float d = dot(qa, qb);
    float c = d < 0 ? -d : d;
    if(c < 0.999) {
        c = acos(c);
        b = 1 / sin(c);
        a = sin(a * c) * b;
        b *= sin(t * c);
        if(d < 0) b = -b;
    }
    return qa * a + qb * b;
}

void main()
{
	//Work out which vertice or two is highest
	float lastHighestVert = -10000000.0; //Assume nothing lower

	for (int i = 0; i < 3; i++)
	{
		float y = gl_in[i].gl_Position.y;
		if (gl_in[i].gl_Position.y >= lastHighestVert)
		{
			lastHighestVert = y;
		}
	}

	//Sample texture of wind at position and clamp to range
	vec4 windTexA = texture(u_WindA, gl_in[0].gl_Position.xy/512.0);
	vec4 windVecA = vec4(windTexA.xyz, 0.0);
	windVecA -= vec4(0.5,0.5,0.5,0.0);
	windVecA *= 6;

	vec4 windTexB = texture(u_WindB, gl_in[0].gl_Position.xy/512.0);
	vec4 windVecB = vec4(windTexB.xyz, 0.0);
	windVecB -= vec4(0.5,0.5,0.5,0.0);
	windVecB *= 6;

	//Mix vectors based on weighting towards A
	vec4 windVec = slerp(windVecB, windVecA, u_WeightA);

	//If wind is close to parallel with leaves
	//Push on bottom additionally
	//(Assume all vertice normals are the same as they should be)
	float result = dot(vec3(windVec.xyz), data_in[0].v_Normal) / (length(windVec)*length(data_in[0].v_Normal));
	float inFlow = 0.0;
	if (result > 1.0 - 0.3 || result < -1.0 + 0.5)
	{
		inFlow = 1.0;
	}

	for (int i = 0; i < 3; i++)
	{
		vec4 position = gl_in[i].gl_Position;

		//If vertex is higher than or equal to last highest
		//Wave tree there
		if (position.y >= lastHighestVert)
		{
			position = position + windVec;
		}
		else
		{
			position = position + inFlow*(windVec / 15.0);
		}

		gl_Position = data_in[i].v_Proj * position;
		v_Normal = data_in[i].v_Normal;
		v_TexCoord = data_in[i].v_TexCoord;
		v_LightSpacePos = u_LightMVP * position;
		v_FragPos = vec3(gl_Position);
		EmitVertex();
	}
	EndPrimitive();
}