#include "game/objects/NPCAi.h"

std::shared_ptr<NPC_OverworldScript> AllocateNPCOvScript(std::string filePath, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<OvSpr_Sprite> npc, std::shared_ptr<OvSpr_RunningSprite> player, GameInput* input)
{
	//Currently working on scripts - TODO - Make work for any sprite and access player properly
	ScriptParse::ScriptWrapper script = ScriptParse::ParseScriptFromText(filePath);
	std::shared_ptr<NPC_OverworldScript> npcScript(new NPC_OverworldScript(script.script, script.size, player.get(), flags, input));
	npcScript->linkText(&textBuff->t1, &textBuff->t2, &textBuff->showTextBox);
	npcScript->linkNPC(std::static_pointer_cast<OvSpr_RunningSprite>(npc)); //if sprite type doesnt support command, undefined behaviour - TODO fix
	return npcScript;
}

NPC_OverworldScript CreateNPCOvScript(std::string filePath, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<OvSpr_Sprite> npc, std::shared_ptr<OvSpr_RunningSprite> player, GameInput* input)
{
	//Currently working on scripts - TODO - Make work for any sprite and access player properly
	ScriptParse::ScriptWrapper script = ScriptParse::ParseScriptFromText(filePath);
	NPC_OverworldScript npcScript(script.script, script.size, player.get(), flags, input);
	npcScript.linkText(&textBuff->t1, &textBuff->t2, &textBuff->showTextBox);
	npcScript.linkNPC(std::static_pointer_cast<OvSpr_RunningSprite>(npc)); //if sprite type doesnt support command, undefined behaviour - TODO fix
	return npcScript;
}

const float NPC_RandWalk::MAX_SEED = 100.0f;
RandomContainer NPC_RandWalk::s_Random;

void NPC_RandWalk::randomWalk()
{
	//Pick direction to walk
	float rnd = s_Random.next();
	if (rnd <= 25.0f)
	{
		*m_Direction = World::Direction::EAST;
		if (canWalk())
		{
			startWalk();
		}
	}
	else if (rnd <= 50.0f)
	{
		*m_Direction = World::Direction::WEST;
		if (canWalk())
		{
			startWalk();
		}
	}
	else if (rnd <= 75.0f)
	{
		*m_Direction = World::Direction::NORTH;
		if (canWalk())
		{
			startWalk();
		}
	}
	else if (rnd <= 100.0f)
	{
		*m_Direction = World::Direction::SOUTH;
		if (canWalk())
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

	if (*m_Busy)
	{
		return;
	}

	if (m_CoolDownTimer <= 0.0f && !*m_Walking && !*m_Busy)
	{
		randomWalk();
	}

	//Walk method
	if (*m_WalkTimer < 1.0 && *m_Walking)
	{
		Ov_Translation::Walk(m_Direction, m_XPos, m_ZPos, m_Sprite, deltaTime, m_WalkTimer);
	}

	//If at end of run or walk cycle
	if (*m_WalkTimer >= 1.0 && *m_Walking)
	{
		cycleEnd();
	}

	//cooldown timer
	if (m_CoolDownTimer > 0.0f)
	{
		m_CoolDownTimer -= deltaTime;
	}
}

void NPC_RandWalk::cycleEnd()
{
	*m_Walking = false;
	*m_WalkTimer = 0.0;

	//Centre on x and y
	Ov_Translation::CentreOnTile(*m_CurrentLevel, *m_WorldLevel, m_XPos, m_YPos, m_ZPos, *m_TileX, *m_TileZ, m_Sprite);
}

bool NPC_RandWalk::canWalk()
{
	World::LevelPermission perm = World::RetrievePermission(*m_CurrentLevel, *m_Direction, { *m_TileX, *m_TileZ }, *m_WorldLevel);
	if (perm.leaving)
	{
		return false;
	}
	//if not leaving, check permissions
	switch (perm.perm)
	{
	case World::MovementPermissions::CLEAR:
		return true;
	default:
		return false;
	}
}

void NPC_RandWalk::startWalk()
{
	*m_Walking = true;
	*m_WalkTimer = 0.0;
	World::ModifyTilePerm(*m_CurrentLevel, *m_Direction, { *m_TileX, *m_TileZ }, *m_WorldLevel);
}