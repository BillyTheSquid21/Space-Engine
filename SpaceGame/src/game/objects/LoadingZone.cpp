#include "game/objects/LoadingZone.h"

void LoadingZoneComponent::render()
{
	if (!m_PlayerInside)
	{
		//Bounds assume north z is negative
		if (m_PlayerPointer->m_XPos >= m_Origin.a && m_PlayerPointer->m_XPos <= m_Origin.a + m_Bounds.a
			&& m_PlayerPointer->m_ZPos <= m_Origin.b && m_PlayerPointer->m_ZPos >= m_Origin.b - m_Bounds.b)
		{
			m_PlayerInside = true;
			m_LoadLv(m_L1_ID);
			m_LoadLv(m_L2_ID);
			EngineLog("Player entered loading zone");
		}
		else
		{
			return;
		}
	}
	else if (m_PlayerInside)
	{
		//Bounds assume north z is negative
		if (!(m_PlayerPointer->m_XPos >= m_Origin.a && m_PlayerPointer->m_XPos <= m_Origin.a + m_Bounds.a
			&& m_PlayerPointer->m_ZPos <= m_Origin.b && m_PlayerPointer->m_ZPos >= m_Origin.b - m_Bounds.b))
		{
			m_PlayerInside = false;
			if (m_PlayerPointer->m_CurrentLevel != m_L1_ID)
			{
				m_UnloadLv(m_L1_ID);
			}
			else
			{
				m_UnloadLv(m_L2_ID);
			}
			EngineLog("Player left loading zone");
			return;
		}
	}

	Struct2f loc = World::Level::queryOrigin(m_PlayerPointer->m_CurrentLevel);
	World::LevelDimensions dim = World::Level::queryDimensions(m_PlayerPointer->m_CurrentLevel);

	if (!(m_PlayerPointer->m_XPos > (loc.a + dim.levelW * World::TILE_SIZE) || m_PlayerPointer->m_XPos < loc.a
		|| m_PlayerPointer->m_ZPos < (loc.b - dim.levelH * World::TILE_SIZE) || m_PlayerPointer->m_ZPos > loc.b))
	{
		return;
	}

	//Switch to other level
	if (m_PlayerPointer->m_CurrentLevel != m_L1_ID)
	{
		m_PlayerPointer->m_CurrentLevel = m_L1_ID;
		EngineLog("Level: ", (int)m_L1_ID);
	}
	else
	{
		m_PlayerPointer->m_CurrentLevel = m_L2_ID;
		EngineLog("Level: ", (int)m_L2_ID);
	}
}