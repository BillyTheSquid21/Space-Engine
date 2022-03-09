#include "game/objects/NPCAi.h"

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
	if (*m_Busy)
	{
		return;
	}

	if (m_CoolDownTimer <= 0.0f && !*m_Walking && !*m_Busy)
	{
		randomWalk();
	}

	//Inherit update method
	TilePosition::update(deltaTime);

	//Walk method
	if (m_WalkTimer < 1.0 && *m_Walking)
	{
		walk(deltaTime);
	}

	//If at end of run or walk cycle
	if (m_WalkTimer >= 1.0 && *m_Walking)
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
	m_WalkTimer = 0.0;

	//Centre on x and y
	Component2f origin = World::Level::queryOrigin(*m_CurrentLevel);
	float expectedX = (float)(World::TILE_SIZE * *m_TileX) + origin.a + World::TILE_SIZE / 2;
	float expectedZ = (float)(-World::TILE_SIZE * *m_TileZ) + origin.b - World::TILE_SIZE / 2;
	float deltaX = *m_XPos - expectedX;
	float deltaZ = *m_ZPos - expectedZ;
	TranslateShape<TextureVertex>(m_Sprite, -deltaX, 0.0f, -deltaZ, Shape::QUAD);
	*m_XPos = expectedX;
	*m_ZPos = expectedZ;
}

bool NPC_RandWalk::canWalk()
{
	World::RetrievePermission perm = World::retrievePermission(*m_CurrentLevel, *m_Direction, { *m_TileX, *m_TileZ });
	if (perm.leaving)
	{
		return false;
	}
	//if not leaving, check permissions
	switch (perm.perm)
	{
	case World::MovementPermissions::WALL:
		return false;
	case World::MovementPermissions::SPRITE_BLOCKING:
		return false;
	default:
		return true;
	}
}

void NPC_RandWalk::startWalk()
{
	*m_Walking = true;
	m_WalkTimer = 0.0;
	World::ModifyTilePerm(*m_CurrentLevel, *m_Direction, { *m_TileX, *m_TileZ });
}

void NPC_RandWalk::walk(double deltaTime)
{
	switch (*m_Direction)
	{
	case World::Direction::EAST:
		*m_XPos += deltaTime * World::TILE_SIZE;
		TranslateShape<TextureVertex>(m_Sprite, deltaTime * World::TILE_SIZE, 0.0f, 0.0f, Shape::QUAD);
		break;
	case World::Direction::WEST:
		*m_XPos += deltaTime * -World::TILE_SIZE;
		TranslateShape<TextureVertex>(m_Sprite, deltaTime * -World::TILE_SIZE, 0.0f, 0.0f, Shape::QUAD);
		break;
	case World::Direction::NORTH:
		*m_ZPos += deltaTime * -World::TILE_SIZE;
		TranslateShape<TextureVertex>(m_Sprite, 0.0f, 0.0f, deltaTime * -World::TILE_SIZE, Shape::QUAD);
		break;
	case World::Direction::SOUTH:
		*m_ZPos += deltaTime * World::TILE_SIZE;
		TranslateShape<TextureVertex>(m_Sprite, 0.0f, 0.0f, deltaTime * World::TILE_SIZE, Shape::QUAD);
		break;
	default:
		break;
	}

	m_WalkTimer += deltaTime;
}