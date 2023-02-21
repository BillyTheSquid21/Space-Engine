#include "renderer/Lighting.h"

void SGRender::Lighting::set(Camera* camera)
{
	//Clean to avoid spillover
	clean();

	//Create SSBO and upload base lighting
	m_LightingSSBO.create();
	updateLightBuffer();

	m_Camera = camera;
	m_Set = true;
}

void SGRender::Lighting::clean()
{
	if (!m_Set)
	{
		return;
	}

	m_Camera = nullptr;
	m_BaseData = BaseLightData();
	m_LightList.clear();
	m_LightIDs.clear();
	m_NextLightID = 0;
	m_LightCount = 0;

	m_Set = false;
}

int32_t SGRender::Lighting::addLight(glm::vec3& pos, float brightness, glm::vec3& color, float radius)
{
	int32_t id = m_NextLightID;
	m_LightList.emplace_back(pos, brightness, color, radius);
	m_LightIDs.emplace_back(id, m_LightCount);

	m_LightCount++;
	m_NextLightID++;

	//for now update buffer for any change
	updateLightBuffer();
	return id;
}

bool SGRender::Lighting::removeLight(int32_t id)
{
	int32_t light_index = -1;
	int32_t id_index = -1;
	for (int i = 0; i < m_LightIDs.size(); i++)
	{
		if (m_LightIDs[i].lightID == id)
		{
			light_index = m_LightIDs[i].lightIndex;
			m_LightIDs.erase(m_LightIDs.begin() + i);
			id_index = i; 
			break;
		}
	}

	if (light_index == -1)
	{
		return false;
	}

	m_LightList.erase(m_LightList.begin() + light_index);
	
	//Update ids above position
	for (int i = id_index; i < m_LightIDs.size(); i++)
	{
		m_LightIDs[i].lightIndex--;
	}

	m_LightCount--;
	//for now update buffer for any change
	updateLightBuffer();
	return true;
}

void SGRender::Lighting::updateLightBuffer()
{
	//Allocate temporary cpu side buffer with all data contiguous
	size_t data_size = sizeof(BaseLightData) + (m_CulledLightList.size() * sizeof(PointLight));
	void* tmp_buffer = malloc(data_size);

	//Then insert elements and upload
	memcpy_s(tmp_buffer, data_size, &m_BaseData, sizeof(BaseLightData));

	if (m_CulledLightList.size() > 0)
	{
		void* point_array = (void*)((size_t)tmp_buffer + sizeof(BaseLightData));
		memcpy_s(point_array, m_CulledLightList.size() * sizeof(PointLight), &m_CulledLightList[0], m_CulledLightList.size() * sizeof(PointLight));
	}

	m_LightingSSBO.bind();
	m_LightingSSBO.bufferFullData(tmp_buffer, data_size);
	m_LightingSSBO.unbind();

	free(tmp_buffer);
}

void SGRender::Lighting::linkShader(SGRender::Shader& shader)
{
	shader.bindToUniformBlock("SG_Lighting", m_LightingSSBO.bindingPoint());
}

float SGRender::Lighting::lightRadius(PointLight& light)
{
	float sizeFactor = light.radius * light.brightness;

	//Get attenuation terms
	float adjustedLin = m_BaseData.pointLinear / sizeFactor;
	float adjustedQua = m_BaseData.pointQuad / sizeFactor;

	//Solve attenuation for cutoff
	//Check discriminant isn't negative
	float disc = (m_BaseData.pointLinear * m_BaseData.pointLinear)
		- 4.0f * (m_BaseData.pointQuad * (m_BaseData.pointConstant - (1.0f / m_BaseData.pointCutoff)));

	if (disc < 0.0)
	{
		EngineLog("Error solving attenuation!");
		return 0.0f;
	}

	//Otherwise solve for largest X
	return (-m_BaseData.pointLinear + sqrt(disc))/(2.0f*m_BaseData.pointQuad);
}

void SGRender::Lighting::cullLights()
{
	//Add any light close to frustum to culled list
	m_CulledLightList.clear();

	std::vector<PointLight> culledLights;
	std::vector<LightID> culledIDs;

	//First list all ids
	//Compare lists, as order doesn't change, assume no sorting needed
	//If both new culled ids and old are same, don't buffer
	//Otherwise, buffer from indexes

	//Add to culled IDs
	for (int i = 0; i < m_LightList.size(); i++)
	{
		if (m_Camera->inFrustum(m_LightList[i].position, m_LightList[i].radius + lightRadius(m_LightList[i])))
		{
			culledIDs.push_back(m_LightIDs[i]);
		}
	}

	//Compare against last cull if same length
	bool different = m_CulledLightIDs.size() != culledIDs.size();
	if (m_CulledLightIDs.size() == culledIDs.size())
	{
		for (int i = 0; i < m_CulledLightIDs.size(); i++)
		{
			if (m_CulledLightIDs[i].lightID != culledIDs[i].lightID)
			{
				different = true;
				break;
			}
		}
	}

	if (different)
	{
		for (int i = 0; i < culledIDs.size(); i++)
		{
			culledLights.push_back(m_LightList[culledIDs[i].lightIndex]);
		}
		m_CulledLightList = std::move(culledLights);
		m_CulledLightIDs = std::move(culledIDs);
		updateLightBuffer();
		EngineLogOk("Culled Lights");
		EngineLog("Light Count: ", m_CulledLightList.size());
	}
}