#include "game/objects/TallGrass.h"

void TallGrassAnimationComponent::render()
{
	for (int i = 0; i < m_GrassLoc->size(); i++)
	{
		World::LevelPermission perm = World::RetrievePermission(*m_LevelID, m_GrassLoc->at(i).loc, m_GrassLoc->at(i).height);
		if (perm.perm != m_GrassLoc->at(i).lastPermission)
		{
			if ((*m_ActiveStates)[i] == (char)true)
			{
				continue;
			}
			(*m_ActiveStates)[i] = (char)true;
			m_GrassLoc->at(i).lastPermission = perm.perm;
		}
	}
}

void TallGrassRenderComponent::addGrass(Struct2f levelOrigin, World::TileLoc tile, World::WorldHeight level, World::LevelID levelID, std::vector<GrassData>* grassLoc, std::vector<char>* states)
{
	if (m_Grass->quadCount >= m_Grass->quads.size())
	{
		EngineLog("Try reserving more space, grass count out of range!");
		return;
	}

	//Dimensions
	float xPos = levelOrigin.a + tile.x * (float)World::TILE_SIZE;
	float zPos = -1 * levelOrigin.b + tile.z * (float)World::TILE_SIZE;
	float yPos = ((int)level / sqrt(2)) * World::TILE_SIZE;

	//Create Sprite
	Norm_Tex_Quad grass = CreateNormalTextureQuad(xPos, yPos, (float)World::TILE_SIZE, (float)World::TILE_SIZE, m_UV.u, m_UV.v, m_UV.width, m_UV.height);

	//Position shape
	AxialRotate<NormalTextureVertex>((NormalTextureVertex*)&grass, { xPos, yPos, 0.0f }, -90.0f, Shape::QUAD, Axis::X);
	Translate<NormalTextureVertex>((NormalTextureVertex*)&grass, 0.0f, 0.1f, -zPos - World::TILE_SIZE, Shape::QUAD);

	//Gen normals
	CalculateQuadNormals((NormalTextureVertex*)&grass);

	m_Grass->quads[m_Grass->quadCount] = grass;
	m_Grass->quadCount++;

	//Add data
	World::LevelPermission perm = World::RetrievePermission(levelID, tile, level);
	grassLoc->push_back({ tile, level, perm.perm });
	states->push_back((char)false);
}

void TallGrassRenderComponent::generateIndices()
{
	GenerateQuadArrayIndices(*m_Grass);
}

void TallGrassEncounterComponent::update(double deltaTime)
{
	//Check if change
	bool change = false;
	for (int i = 0; i < m_ActiveStates->size(); i++)
	{
		if (m_ActiveStates->at(i) != m_LastState[i])
		{
			change = true;
			std::copy(m_ActiveStates->begin(), m_ActiveStates->end(), m_LastState.begin());
			break;
		}
	}
	if (!change)
	{
		return;
	}

	float roll = m_Random.next();
	if (roll > 3)
	{
		return;
	}
	m_Battle();
}