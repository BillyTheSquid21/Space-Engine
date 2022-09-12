#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 u_LightMVP;
uniform sampler2D u_WindA;
uniform sampler2D u_WindB;
uniform float u_WeightA;
uniform vec3 u_PlayerPos;

in DATA
{
	vec3 v_Normal;
	vec4 v_Color;
	mat4 v_Proj;
	float v_HeightVariance;
} data_in[];

out vec3 v_Normal;
out vec3 v_FragPos;
out vec4 v_LightSpacePos;
out vec4 v_Color;

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
	float lastLowestVert = 10000000.0;
	for (int i = 0; i < 3; i++)
	{
		float y = gl_in[i].gl_Position.y;
		if (gl_in[i].gl_Position.y >= lastHighestVert)
		{
			lastHighestVert = y;
		}
		else if (gl_in[i].gl_Position.y <= lastLowestVert)
		{
			lastLowestVert = y;
		}
	}

	//Sample texture of wind at position and clamp to range
	vec4 windTexA = texture(u_WindA, gl_in[0].gl_Position.xy/512.0);
	vec4 windVecA = vec4(windTexA.xyz, 0.0);
	windVecA -= vec4(0.5,0.5,0.5,0.0);
	windVecA *= 12;

	vec4 windTexB = texture(u_WindB, gl_in[0].gl_Position.xy/512.0);
	vec4 windVecB = vec4(windTexB.xyz, 0.0);
	windVecB -= vec4(0.5,0.5,0.5,0.0);
	windVecB *= 12;

	//Mix vectors based on weighting towards A
	vec4 windVec = slerp(windVecB, windVecA, u_WeightA);

	//If wind is close to parallel with leaves
	//Push on bottom additionally
	//(Assume all vertice normals are the same as they should be)
	float result = dot(vec3(windVec.xyz), data_in[0].v_Normal) / (length(windVec)*length(data_in[0].v_Normal));
	float inFlow = 1.0/result;

	//If close to player, move in opposite direction
	float underfootLowerThreshold = 6.5;
	float underfootUpperThreshold = 22.0;

	vec2 playerToGrass = vec2(gl_in[2].gl_Position.xz) - vec2(u_PlayerPos.xz);
	vec3 playerDisplace = vec3(0.0,0.0,0.0);
	float dist = length(playerToGrass);
	if (dist < underfootUpperThreshold && gl_in[2].gl_Position.y == u_PlayerPos.y)
	{
		//Push away and reduce strength with distance
	    vec2 normalizedDist = normalize(playerToGrass);
		playerDisplace = vec3(normalizedDist.x, 0.0, normalizedDist.y);
		playerDisplace *= (vec3(180, 1.0, 180)/pow((length(playerToGrass))*(float((dist > 3.5))), 2));
	}

	for (int i = 0; i < 3; i++)
	{
		vec4 position = gl_in[i].gl_Position;

		//If vertex is higher than or equal to last highest
		//Wave tree there
		if (position.y >= lastHighestVert)
		{
			position = position + windVec;
			position += vec4(0.0, data_in[i].v_HeightVariance, 0.0, 0.0);
			
			//Flatten if underfoot
			playerDisplace.y = float((dist < underfootLowerThreshold)) * (lastLowestVert - position.y);
			playerDisplace *= vec3(dist >= underfootLowerThreshold, 1.0 ,dist >= underfootLowerThreshold);
			position += vec4(playerDisplace.xyz, 0.0);
		}

		gl_Position = data_in[i].v_Proj * position;

		//Calculate new normal vector
		v_Normal = normalize(vec3(windVec));

		//Pass data
		v_LightSpacePos = u_LightMVP * position;
		v_FragPos = vec3(gl_Position);
		v_Color = data_in[i].v_Color;
		EmitVertex();
	}
	EndPrimitive();
}