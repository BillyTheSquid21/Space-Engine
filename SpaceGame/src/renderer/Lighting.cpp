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
	size_t data_size = sizeof(BaseLightData) + (m_LightCount * sizeof(PointLight));
	void* tmp_buffer = malloc(data_size);

	//Then insert elements and upload
	memcpy_s(tmp_buffer, data_size, &m_BaseData, sizeof(BaseLightData));

	if (m_LightCount)
	{
		void* point_array = (void*)((size_t)tmp_buffer + sizeof(BaseLightData));
		memcpy_s(point_array, m_LightCount * sizeof(PointLight), &m_LightList[0], m_LightCount * sizeof(PointLight));
	}

	m_LightingSSBO.bind();
	m_LightingSSBO.bufferData(tmp_buffer, 0, data_size);
	m_LightingSSBO.unbind();

	free(tmp_buffer);
}

void SGRender::Lighting::linkShader(SGRender::Shader& shader)
{
	shader.bindToUniformBlock("SG_Lighting", m_LightingSSBO.bindingPoint());
}