#include "game/objects/NPCAi.h"

std::shared_ptr<NPC_OverworldScript> AllocateNPCOvScript(std::string filePath, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<Ov_Sprite::Sprite> npc, std::shared_ptr<Ov_Sprite::RunSprite> player)
{
	//Currently working on scripts - TODO - Make work for any sprite and access player properly
	ScriptParse::ScriptWrapper script = ScriptParse::ParseScriptFromText(filePath);
	std::shared_ptr<NPC_OverworldScript> npcScript(new NPC_OverworldScript(script.script, script.size, player));
	npcScript->linkText(textBuff);
	npcScript->linkNPC(std::static_pointer_cast<Ov_Sprite::RunSprite>(npc)); //if sprite type doesnt support command, undefined behaviour - TODO fix
	return npcScript;
}

NPC_OverworldScript CreateNPCOvScript(std::string filePath, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<Ov_Sprite::Sprite> npc, std::shared_ptr<Ov_Sprite::RunSprite> player)
{
	//Currently working on scripts - TODO - Make work for any sprite and access player properly
	ScriptParse::ScriptWrapper script = ScriptParse::ParseScriptFromText(filePath);
	NPC_OverworldScript npcScript(script.script, script.size, player);
	npcScript.linkText(textBuff);
	npcScript.linkNPC(std::static_pointer_cast<Ov_Sprite::RunSprite>(npc)); //if sprite type doesnt support command, undefined behaviour - TODO fix
	return npcScript;
}

const float NPC_RandWalk::MAX_SEED = 100.0f;
SGRandom::RandomContainer NPC_RandWalk::s_Random;

void NPC_RandWalk::randomWalk()
{
	//Pick direction to walk
	float rnd = s_Random.next();
	if (rnd <= 25.0f)
	{
		m_NPC->m_Direction = World::Direction::EAST;
		if (Ov_Translation::CheckCanWalkNPC(m_NPC.get()))
		{
			startWalk();
		}
	}
	else if (rnd <= 50.0f)
	{
		m_NPC->m_Direction = World::Direction::WEST;
		if (Ov_Translation::CheckCanWalkNPC(m_NPC.get()))
		{
			startWalk();
		}
	}
	else if (rnd <= 75.0f)
	{
		m_NPC->m_Direction = World::Direction::NORTH;
		if (Ov_Translation::CheckCanWalkNPC(m_NPC.get()))
		{
			startWalk();
		}
	}
	else if (rnd <= 100.0f)
	{
		m_NPC->m_Direction = World::Direction::SOUTH;
		if (Ov_Translation::CheckCanWalkNPC(m_NPC.get()))
		{
			startWalk();
		}
	}

	//Pick cooldown timer
	m_CoolDownTimer = s_Random.next() / 8.0f;
}

void NPC_RandWalk::update(double deltaTime)
{
	//Inherit update method
	TilePosition::update(deltaTime);

	if (m_NPC->m_Busy)
	{
		return;
	}

	if (m_CoolDownTimer <= 0.0f && !m_NPC->m_Walking && !m_NPC->m_Busy)
	{
		randomWalk();
	}

	//Walk method
	Ov_Translation::SpriteWalk(m_NPC.get(), deltaTime);

	//cooldown timer
	if (m_CoolDownTimer > 0.0f)
	{
		m_CoolDownTimer -= deltaTime;
	}
}

void NPC_RandWalk::cycleEnd()
{
	m_NPC->m_Walking = false;
	m_NPC->m_Timer = 0.0;

	//Centre on x and y
	Ov_Translation::CentreOnTile(m_NPC->m_CurrentLevel, m_NPC->m_WorldLevel, &m_NPC->m_XPos, &m_NPC->m_YPos, &m_NPC->m_ZPos, m_NPC->m_Tile, &m_NPC->m_Sprite);
}

void NPC_RandWalk::startWalk()
{
	m_NPC->m_Walking = true;
	m_NPC->m_Timer = 0.0;
	World::ModifyTilePerm(m_NPC->m_CurrentLevel, m_NPC->m_Direction, m_NPC->m_Tile, m_NPC->m_WorldLevel, m_NPC->m_LastPermission, m_NPC->m_LastPermissionPtr);
}