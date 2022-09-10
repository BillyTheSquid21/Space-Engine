#include "game/objects/OverworldSprite.h"

//components
void Ov_Sprite::SpriteRender::render()
{
	using namespace Geometry;
	if (m_Sprite != nullptr && m_Renderer != nullptr) {
		m_Renderer->commit((SGRender::NTVertex*)m_Sprite, GetFloatCount<SGRender::NTVertex>(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT);
	}
}

void Ov_Sprite::TilePosition::update(double deltaTime)
{
	//Works out current tile
	Struct2f origin = World::Level::queryOrigin(*m_CurrentLevel);
	float deltaX = *m_XPos - origin.a;
	float deltaZ = *m_ZPos - origin.b;
	m_Tile->x = (unsigned int)(deltaX / World::TILE_SIZE);
	m_Tile->z = (unsigned int)(-deltaZ / World::TILE_SIZE);
}

void Ov_Sprite::SpriteMap::update(double deltaTime)
{
	if (m_LastOffX != *m_OffsetX || m_LastOffY != *m_OffsetY)
	{
		//Update sprite map
		TileUV data = m_Map->uvTile(m_TextureOrigin.textureX + *m_OffsetX, m_TextureOrigin.textureY + *m_OffsetY);
		Geometry::SetQuadUV((SGRender::NTVertex*)m_Sprite, data.u, data.v, data.width, data.height);
		m_LastOffX = *m_OffsetX; m_LastOffY = *m_OffsetY;
	}
}

void Ov_Sprite::UpdateAnimationFacing::update(double deltaTime)
{
	if (*m_Direction != m_LastDirection)
	{
		//(East is default)
		switch (*m_Direction)
		{
		case World::Direction::WEST:
			*m_OffsetY = 1;
			break;
		case World::Direction::SOUTH:
			*m_OffsetY = 2;
			break;
		case World::Direction::NORTH:
			*m_OffsetY = 3;
			break;
		default:
			*m_OffsetY = 0;
			break;
		}

		m_LastDirection = *m_Direction;
	}
}

void Ov_Sprite::UpdateAnimationWalking::update(double deltaTime)
{
	UpdateAnimationFacing::update(deltaTime);

	if (*m_Walking)
	{
		using World::WALK_FRAME_DURATION;
		if (m_Timer <= WALK_FRAME_DURATION)
		{
			*m_OffsetX = 1;
		}
		else if (m_Timer > WALK_FRAME_DURATION && m_Timer <= 2*WALK_FRAME_DURATION)
		{
			*m_OffsetX = 0;
		}
		else if (m_Timer > 2*WALK_FRAME_DURATION && m_Timer <= 3*WALK_FRAME_DURATION)
		{
			*m_OffsetX = 2;
		}
		else if (m_Timer > 3*WALK_FRAME_DURATION && m_Timer <= 4*WALK_FRAME_DURATION)
		{
			*m_OffsetX = 0;
		}
		else
		{
			*m_OffsetX = 1;
			m_Timer = 0.0f;
		}
		m_Timer += deltaTime;
		m_WasWalking = true;
	}
	else if (m_WasWalking)
	{
		*m_OffsetX = 0;
		m_Timer = 0.0f;
		m_WasWalking = false;
	}
}

void Ov_Sprite::UpdateAnimationRunning::update(double deltaTime)
{
	UpdateAnimationWalking::update(deltaTime);
	if (*m_Running)
	{
		using World::RUN_FRAME_DURATION;
		if (m_Timer <= RUN_FRAME_DURATION)
		{
			*m_OffsetX = 4;
		}
		else if (m_Timer > RUN_FRAME_DURATION && m_Timer <= 2 * RUN_FRAME_DURATION)
		{
			*m_OffsetX = 3;
		}
		else if (m_Timer > 2 * RUN_FRAME_DURATION && m_Timer <= 3 * RUN_FRAME_DURATION)
		{
			*m_OffsetX = 5;
		}
		else if (m_Timer > 3 * RUN_FRAME_DURATION && m_Timer <= 4 * RUN_FRAME_DURATION)
		{
			*m_OffsetX = 3;
		}
		else
		{
			*m_OffsetX = 4;
			m_Timer = 0.0f;
		}
		m_Timer += deltaTime;
		m_WasRunning = true;
		return;
	}

	if (m_WasRunning)
	{
		*m_OffsetX = 3;
		m_Timer = 0.0f;
		m_WasRunning = false;
		m_ResetToZero = true;
		return;
	}

	//Final check to avoid modifying every frame
	if (m_ResetToZero)
	{
		*m_OffsetX = 0;
		m_ResetToZero = false;
	}
}

//Objects
Ov_Sprite::Sprite::Sprite(SpriteData data, bool block)
{
	using namespace World; using namespace Geometry; using namespace SGRender;
	//Set
	m_Tile = data.tile; m_CurrentLevel = data.levelID;
	//X Y Z
	//Get origin
	Struct2f origin = World::Level::queryOrigin(data.levelID);
	float x = origin.a + (m_Tile.x * TILE_SIZE); float y = ((float)data.height / sqrt(2)) * TILE_SIZE;
	float z = origin.b -(m_Tile.z * TILE_SIZE);
	m_WorldLevel = data.height;
	//Set pos - x in tile middle
	m_XPos = x + TILE_SIZE / 2; m_YPos = y; m_ZPos = z - TILE_SIZE / 2;
	//Make Sprite
	m_Sprite = CreateNormalTextureQuad(x, y + TILE_SIZE, TILE_SIZE, TILE_SIZE, 0.0f, 0.0f, 0.05f, 0.1f);
	//Position Sprite
	AxialRotate<NTVertex>((NTVertex*)&m_Sprite, { x + TILE_SIZE / 2, y, 0.0f }, -45.0f, Shape::QUAD, Axis::X);
	Translate<NTVertex>((NTVertex*)&m_Sprite, 0.0f, 0.0f, m_ZPos, Shape::QUAD);
	//Calc normal
	CalculateQuadNormals((NTVertex*)&m_Sprite);

	//Make current tile blocked
	if (block)
	{
		World::Level::PermVectorFragment perm = Level::queryPermissions(m_CurrentLevel, data.height);
		World::LevelDimensions dim = Level::queryDimensions(m_CurrentLevel);
		unsigned int tileLocation = m_Tile.x * dim.levelH + m_Tile.z;
		perm.pointer[tileLocation] = MovementPermissions::SPRITE_BLOCKING;
	}
}

void Ov_Sprite::Sprite::setSprite(TileUV data)
{
	Geometry::SetQuadUV((SGRender::NTVertex*)&m_Sprite, data.u, data.v, data.width, data.height);
}

//Translation
void Ov_Translation::Walk(World::Direction* direction, float* x, float* z, Norm_Tex_Quad* sprite, double deltaTime, double* walkTimer)
{
	using namespace World; using namespace Geometry; using namespace SGRender;
	switch (*direction)
	{
	case Direction::EAST:
		*x += deltaTime * TILE_SIZE * WALK_SPEED;
		Translate<NTVertex>(sprite, deltaTime * TILE_SIZE * WALK_SPEED, 0.0f, 0.0f, Shape::QUAD);
		break;
	case Direction::WEST:
		*x += deltaTime * -TILE_SIZE * WALK_SPEED;
		Translate<NTVertex>(sprite, deltaTime * -TILE_SIZE * WALK_SPEED, 0.0f, 0.0f, Shape::QUAD);
		break;
	case Direction::NORTH:
		*z += deltaTime * -TILE_SIZE * WALK_SPEED;
		Translate<NTVertex>(sprite, 0.0f, 0.0f, deltaTime * -TILE_SIZE * WALK_SPEED, Shape::QUAD);
		break;
	case Direction::SOUTH:
		*z += deltaTime * TILE_SIZE * WALK_SPEED;
		Translate<NTVertex>(sprite, 0.0f, 0.0f, deltaTime * TILE_SIZE * WALK_SPEED, Shape::QUAD);
		break;
	default:
		break;
	}

	*walkTimer += deltaTime;
}

void Ov_Translation::Run(World::Direction* direction, float* x, float* z, Norm_Tex_Quad* sprite, double deltaTime, double* walkTimer)
{
	using namespace World; using namespace Geometry; using namespace SGRender;
	switch (*direction)
	{
	case Direction::EAST:
		*x += deltaTime * TILE_SIZE * RUN_SPEED;
		Translate<NTVertex>(sprite, deltaTime * TILE_SIZE * RUN_SPEED, 0.0f, 0.0f, Shape::QUAD);
		break;
	case Direction::WEST:
		*x += deltaTime * -TILE_SIZE * RUN_SPEED;
		Translate<NTVertex>(sprite, deltaTime * -TILE_SIZE * RUN_SPEED, 0.0f, 0.0f, Shape::QUAD);
		break;
	case Direction::NORTH:
		*z += deltaTime * -TILE_SIZE * RUN_SPEED;
		Translate<NTVertex>(sprite, 0.0f, 0.0f, deltaTime * -TILE_SIZE * RUN_SPEED, Shape::QUAD);
		break;
	case Direction::SOUTH:
		*z += deltaTime * TILE_SIZE * RUN_SPEED;
		Translate<NTVertex>(sprite, 0.0f, 0.0f, deltaTime * TILE_SIZE * RUN_SPEED, Shape::QUAD);
		break;
	default:
		break;
	}

	*walkTimer += deltaTime;
}

void Ov_Translation::CentreOnTile(World::LevelID currentLevel, World::WorldHeight worldLevel, float* x, float* y, float* z, World::Tile tile, Norm_Tex_Quad* sprite)
{
	//Centre on x and y
	using namespace Geometry;
	Struct2f origin = World::Level::queryOrigin(currentLevel);
	float expectedX = (float)(World::TILE_SIZE * tile.x) + origin.a + World::TILE_SIZE / 2;
	float expectedZ = (float)(-World::TILE_SIZE * tile.z) + origin.b - World::TILE_SIZE / 2;
	float expectedY = ((int)worldLevel / sqrt(2)) * World::TILE_SIZE;
	float deltaX = *x - expectedX;
	float deltaZ = *z - expectedZ;
	float deltaY = *y - expectedY;
	Translate<SGRender::NTVertex>(sprite, -deltaX, -deltaY, -deltaZ, Shape::QUAD);
	*x = expectedX;
	*z = expectedZ;
	*y = expectedY;
}

void Ov_Translation::CentreOnTile(World::LevelID currentLevel, World::WorldHeight worldLevel, float* x, float* y, float* z, World::Tile tile, Norm_Tex_Quad* sprite, bool onSlope)
{
	//Centre on x and y
	using namespace Geometry;
	Struct2f origin = World::Level::queryOrigin(currentLevel);
	float expectedX = (float)(World::TILE_SIZE * tile.x) + origin.a + World::TILE_SIZE / 2;
	float expectedZ = (float)(-World::TILE_SIZE * tile.z) + origin.b - World::TILE_SIZE / 2;
	float expectedY = ((int)worldLevel / sqrt(2)) * World::TILE_SIZE;
	if (onSlope)
	{
		expectedY = ((int)worldLevel / (sqrt(2))) * World::TILE_SIZE - ((int)1 / (2 * (sqrt(2)))) * World::TILE_SIZE;
	}
	float deltaX = *x - expectedX;
	float deltaZ = *z - expectedZ;
	float deltaY = *y - expectedY;
	Translate<SGRender::NTVertex>(sprite, -deltaX, -deltaY, -deltaZ, Shape::QUAD);
	*x = expectedX;
	*z = expectedZ;
	*y = expectedY;
}

void Ov_Translation::AscendSlope(float* y, Norm_Tex_Quad* sprite, double deltaTime, bool running)
{
	using namespace Geometry;
	if (!running)
	{
		*y += (World::WALK_SPEED / sqrt(2))* World::TILE_SIZE * deltaTime;
		Translate<SGRender::NTVertex>(sprite, 0.0f, (World::WALK_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime, 0.0f, Shape::QUAD);
	}
	else
	{
		*y += (World::RUN_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime;
		Translate<SGRender::NTVertex>(sprite, 0.0f, (World::RUN_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime, 0.0f, Shape::QUAD);
	}
}

void Ov_Translation::AscendSlope(float* y, Norm_Tex_Quad* sprite, double deltaTime, bool running, double timer, bool verticalFirst
)
{
	constexpr double WALK_MAX = World::WALK_DURATION / 2;
	constexpr double RUN_MAX = World::RUN_DURATION / 2;
	if (!running)
	{
		if (!verticalFirst && timer < WALK_MAX)
		{
			return;
		}
		else if (verticalFirst && timer > WALK_MAX)
		{
			return;
		}
	}
	else if (running)
	{
		if (!verticalFirst && timer < RUN_MAX)
		{
			return;
		}
		else if (verticalFirst && timer > RUN_MAX)
		{
			return;
		}
	}
	AscendSlope(y, sprite, deltaTime, running);
}

void Ov_Translation::DescendSlope(float* y, Norm_Tex_Quad* sprite, double deltaTime, bool running)
{
	using namespace Geometry;
	if (!running)
	{
		*y += (-World::WALK_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime;
		Translate<SGRender::NTVertex>(sprite, 0.0f, (-World::WALK_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime, 0.0f, Shape::QUAD);
	}
	else
	{
		*y += (-World::RUN_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime;
		Translate<SGRender::NTVertex>(sprite, 0.0f, (-World::RUN_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime, 0.0f, Shape::QUAD);
	}
}

void Ov_Translation::DescendSlope(float* y, Norm_Tex_Quad* sprite, double deltaTime, bool running, double timer, bool verticalFirst)
{
	constexpr double WALK_MAX = World::WALK_DURATION / 2;
	constexpr double RUN_MAX = World::RUN_DURATION / 2;
	if (!running)
	{
		if (!verticalFirst && timer < WALK_MAX)
		{
			return;
		}
		else if (verticalFirst && timer > WALK_MAX)
		{
			return;
		}
	}
	else if (running)
	{
		if (!verticalFirst && timer < RUN_MAX)
		{
			return;
		}
		else if (verticalFirst && timer > RUN_MAX)
		{
			return;
		}
	}
	DescendSlope(y, sprite, deltaTime, running);
}

void Ov_Translation::CheckAscend(Ov_Sprite::WalkSprite* spr)
{
	//Check how to ascend
	spr->m_MoveVerticalFirst = false;
	if (spr->m_Ascend)
	{
		if (spr->m_CurrentIsSlope && !spr->m_NextIsSlope)
		{
			spr->m_MoveVerticalFirst = true;
		}
		if (spr->m_CurrentIsSlope && spr->m_NextIsSlope)
		{
			spr->m_MoveVerticalFirst = false;
		}
	}
}

void Ov_Translation::WalkMethod(Ov_Sprite::WalkSprite* spr, double deltaTime)
{
	//Walk method
	if (spr->m_Timer < World::WALK_DURATION && spr->m_Walking)
	{
		Ov_Translation::Walk(&spr->m_Direction, &spr->m_XPos, &spr->m_ZPos, &spr->m_Sprite, deltaTime, &spr->m_Timer);
		if (spr->m_Ascend == 1)
		{
			if (spr->m_CurrentIsSlope && spr->m_NextIsSlope)
			{
				Ov_Translation::AscendSlope(&spr->m_YPos, &spr->m_Sprite, deltaTime, false);
			}
			else
			{
				Ov_Translation::AscendSlope(&spr->m_YPos, &spr->m_Sprite, deltaTime, false, spr->m_Timer, spr->m_MoveVerticalFirst);
			}
		}
		else if (spr->m_Ascend == -1)
		{
			if (spr->m_CurrentIsSlope && spr->m_NextIsSlope)
			{
				Ov_Translation::DescendSlope(&spr->m_YPos, &spr->m_Sprite, deltaTime, false);
			}
			else
			{
				Ov_Translation::DescendSlope(&spr->m_YPos, &spr->m_Sprite, deltaTime, false, spr->m_Timer, spr->m_MoveVerticalFirst);
			}
		}
	}
}

void Ov_Translation::RunMethod(Ov_Sprite::RunSprite* spr, double deltaTime)
{
	if (!(spr->m_Timer < World::RUN_DURATION && spr->m_Running))
	{
		return;
	}

	Ov_Translation::Run(&spr->m_Direction, &spr->m_XPos, &spr->m_ZPos, &spr->m_Sprite, deltaTime, &spr->m_Timer);
	if (spr->m_Ascend == 1)
	{
		if (spr->m_CurrentIsSlope && spr->m_NextIsSlope)
		{
			Ov_Translation::AscendSlope(&spr->m_YPos, &spr->m_Sprite, deltaTime, true);
		}
		else
		{
			Ov_Translation::AscendSlope(&spr->m_YPos, &spr->m_Sprite, deltaTime, true, spr->m_Timer, spr->m_MoveVerticalFirst);
		}
	}
	else if (spr->m_Ascend == -1)
	{
		if (spr->m_CurrentIsSlope && spr->m_NextIsSlope)
		{
			Ov_Translation::DescendSlope(&spr->m_YPos, &spr->m_Sprite, deltaTime, true);
		}
		else
		{
			Ov_Translation::DescendSlope(&spr->m_YPos, &spr->m_Sprite, deltaTime, true, spr->m_Timer, spr->m_MoveVerticalFirst);
		}
	}
}

void Ov_Translation::CycleEnd(Ov_Sprite::WalkSprite* spr)
{
	spr->m_Walking = false;
	spr->m_Timer = 0.0;

	CentreSprite(spr);
}

void Ov_Translation::CycleEnd(Ov_Sprite::RunSprite* spr)
{
	spr->m_Walking = false;
	spr->m_Running = false;
	spr->m_Timer = 0.0;

	CentreSprite(spr);
}

void Ov_Translation::CentreSprite(Ov_Sprite::WalkSprite* spr)
{
	//Centre on x and y
	if (!spr->m_NextIsSlope)
	{
		//Is false as 
		Ov_Translation::CentreOnTile(spr->m_CurrentLevel, spr->m_WorldLevel, &spr->m_XPos, &spr->m_YPos, &spr->m_ZPos, spr->m_Tile, &spr->m_Sprite, false);
		return;
	}
	if (spr->m_NextIsSlope)
	{
		//Is true as 
		Ov_Translation::CentreOnTile(spr->m_CurrentLevel, spr->m_WorldLevel, &spr->m_XPos, &spr->m_YPos, &spr->m_ZPos, spr->m_Tile, &spr->m_Sprite, true);
		return;
	}
}

bool Ov_Translation::SpriteWalk(Ov_Sprite::WalkSprite* spr, double deltaTime)
{
	CheckAscend(spr);
	WalkMethod(spr, deltaTime);
	if (spr->m_Timer >= World::WALK_DURATION && spr->m_Walking)
	{
		CycleEnd(spr);
		return false;
	}
	return true;
}

bool Ov_Translation::SpriteRun(Ov_Sprite::RunSprite* spr, double deltaTime)
{
	if (!spr->m_Running)
	{
		return false;
	}

	CheckAscend(spr);
	RunMethod(spr, deltaTime);
	if (spr->m_Timer >= World::RUN_DURATION)
	{
		CycleEnd(spr);
		return false;
	}
	return true;
}

bool Ov_Translation::CheckCanWalk(Ov_Sprite::WalkSprite* spr)
{
	//Check if leaving level
	World::LevelPermission permissionNext = World::RetrievePermission(spr->m_CurrentLevel, spr->m_Direction, spr->m_Tile, spr->m_WorldLevel);
	if (permissionNext.leaving)
	{
		if (spr->m_LastPermission == World::MovementPermissions::LEVEL_BRIDGE)
		{
			return true;
		}
	}

	World::Direction direction = spr->m_Direction; World::LevelID levelID = spr->m_CurrentLevel;
	World::WorldHeight worldLevel = spr->m_WorldLevel;
	spr->m_Ascend = 0; spr->m_CurrentIsSlope = false; spr->m_NextIsSlope = false;
	
	//Check if next tile is impassable - check is here to prevent changing data like slope
	switch (permissionNext.perm)
	{
	case World::MovementPermissions::WALL:
		return false;
	case World::MovementPermissions::SPRITE_BLOCKING:
		return false;
	default:
		break;
	}

	//Check current permission if relevant
	switch (spr->m_LastPermission)
	{
		//Stairs
	case World::MovementPermissions::STAIRS_NORTH:
		spr->m_CurrentIsSlope = true;
		if (direction == World::Direction::NORTH)
		{
			spr->m_Ascend = 1;
		}
		else if (direction == World::Direction::SOUTH)
		{
			spr->m_Ascend = -1;
			int levelTmp = (int)worldLevel;
			levelTmp--;
			spr->m_WorldLevel = (World::WorldHeight)levelTmp;
		}
		break;
	case World::MovementPermissions::STAIRS_SOUTH:
		spr->m_CurrentIsSlope = true;
		if (direction == World::Direction::SOUTH)
		{
			spr->m_Ascend = 1;
		}
		else if (direction == World::Direction::NORTH)
		{
			spr->m_Ascend = -1;
			int levelTmp = (int)worldLevel;
			levelTmp--;
			spr->m_WorldLevel = (World::WorldHeight)levelTmp;
		}
		break;
	case World::MovementPermissions::STAIRS_EAST:
		spr->m_CurrentIsSlope = true;
		if (direction == World::Direction::EAST)
		{
			spr->m_Ascend = 1;
		}
		else if (direction == World::Direction::WEST)
		{
			spr->m_Ascend = -1;
			int levelTmp = (int)worldLevel;
			levelTmp--;
			spr->m_WorldLevel = (World::WorldHeight)levelTmp;
		}
		break;
	case World::MovementPermissions::STAIRS_WEST:
		spr->m_CurrentIsSlope = true;
		if (direction == World::Direction::WEST)
		{
			spr->m_Ascend = 1;
		}
		else if (direction == World::Direction::EAST)
		{
			spr->m_Ascend = -1;
			int levelTmp = (int)worldLevel;
			levelTmp--;
			spr->m_WorldLevel = (World::WorldHeight)levelTmp;
		}
		break;
	}

	//Check next permission if relevant
	switch (permissionNext.perm)
	{
	case World::MovementPermissions::STAIRS_NORTH:
	{
		spr->m_NextIsSlope = true;
		if (direction == World::Direction::NORTH)
		{
			spr->m_Ascend = 1;
			int levelTmp = (int)worldLevel;
			levelTmp++;
			spr->m_WorldLevel = (World::WorldHeight)levelTmp;
			return true;
		}
		else if (direction == World::Direction::SOUTH)
		{
			spr->m_Ascend = -1;
			return true;
		}
		return true;
	}
	case World::MovementPermissions::STAIRS_SOUTH:
	{
		spr->m_NextIsSlope = true;
		if (direction == World::Direction::SOUTH)
		{
			spr->m_Ascend = 1;
			int levelTmp = (int)worldLevel;
			levelTmp++;
			spr->m_WorldLevel = (World::WorldHeight)levelTmp;
			return true;
		}
		else if (direction == World::Direction::NORTH)
		{
			spr->m_Ascend = -1;
			return true;
		}
		return true;
	}
	case World::MovementPermissions::STAIRS_EAST:
	{
		spr->m_NextIsSlope = true;
		if (direction == World::Direction::EAST)
		{
			spr->m_Ascend = 1;
			int levelTmp = (int)worldLevel;
			levelTmp++;
			spr->m_WorldLevel = (World::WorldHeight)levelTmp;
			return true;
		}
		else if (direction == World::Direction::WEST)
		{
			spr->m_Ascend = -1;
			return true;
		}
		return true;
	}
	case World::MovementPermissions::STAIRS_WEST:
	{
		spr->m_NextIsSlope = true;
		if (direction == World::Direction::WEST)
		{
			spr->m_Ascend = 1;
			int levelTmp = (int)worldLevel;
			levelTmp++;
			spr->m_WorldLevel = (World::WorldHeight)levelTmp;
			return true;
		}
		else if (direction == World::Direction::EAST)
		{
			spr->m_Ascend = -1;
			return true;
		}
		return true;
	}
	default:
		return true;
	}
}

bool Ov_Translation::CheckCanWalkNPC(Ov_Sprite::WalkSprite* spr)
{
	World::Direction direction = spr->m_Direction; World::LevelID levelID = spr->m_CurrentLevel;
	World::WorldHeight worldLevel = spr->m_WorldLevel;
	World::LevelPermission permissionNext = World::RetrievePermission(levelID, direction, spr->m_Tile, spr->m_WorldLevel);
	spr->m_Ascend = 0; spr->m_CurrentIsSlope = false; spr->m_NextIsSlope = false;

	//Check if next tile is impassable
	switch (permissionNext.perm)
	{
	case World::MovementPermissions::CLEAR:
		return true;
	default:
		return false;
	}
}

std::shared_ptr<Ov_Sprite::Sprite> Ov_ObjCreation::BuildSprite(Ov_Sprite::SpriteData data, TileMap& map, SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* renGrp,
	SGRender::Renderer* sprtRen, bool block)
{
	std::shared_ptr<Ov_Sprite::Sprite> sprite(new Ov_Sprite::Sprite(data, block));
	sprite->setSprite(map.uvTile(data.texture.textureX, data.texture.textureY));
	//Add render group components
	renGrp->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, sprtRen);
	return sprite;
}

std::shared_ptr<Ov_Sprite::DirectionSprite> Ov_ObjCreation::BuildDirectionalSprite(Ov_Sprite::SpriteData data, TileMap& map, SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* renGrp,
	SGObject::UpdateCompGroup<Ov_Sprite::SpriteMap>* sprMap, SGObject::UpdateCompGroup<Ov_Sprite::UpdateAnimationFacing>* faceUp, SGRender::Renderer* sprtRen)
{
	std::shared_ptr<Ov_Sprite::DirectionSprite> sprite(new Ov_Sprite::DirectionSprite(data, true));
	sprite->setSprite(map.uvTile(data.texture.textureX, data.texture.textureY));
	//Add render group components
	renGrp->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, sprtRen);
	//Add sprite map components
	sprMap->addComponent(&sprite->m_UpdateComps, &sprite->m_Sprite, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &map, data.texture);
	//Add facing map components
	faceUp->addComponent(&sprite->m_UpdateComps, &sprite->m_AnimationOffsetY, &sprite->m_Direction);
	return sprite;
}

std::shared_ptr<Ov_Sprite::WalkSprite> Ov_ObjCreation::BuildWalkingSprite(Ov_Sprite::SpriteData data, TileMap& map, SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* renGrp,
	SGObject::UpdateCompGroup<Ov_Sprite::SpriteMap>* sprMap, SGObject::UpdateCompGroup<Ov_Sprite::UpdateAnimationWalking>* walkUp, SGRender::Renderer* sprtRen)
{
	std::shared_ptr<Ov_Sprite::WalkSprite> sprite(new Ov_Sprite::WalkSprite(data, true));
	sprite->setSprite(map.uvTile(data.texture.textureX, data.texture.textureY));
	//Add render group components
	renGrp->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, sprtRen);
	//Add sprite map components
	sprMap->addComponent(&sprite->m_UpdateComps, &sprite->m_Sprite, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &map, data.texture);
	//Add walking map components
	walkUp->addComponent(&sprite->m_UpdateComps, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &sprite->m_Direction, &sprite->m_Walking);
	return sprite;
}

std::shared_ptr<Ov_Sprite::RunSprite> Ov_ObjCreation::BuildRunningSprite(Ov_Sprite::SpriteData data, TileMap& map, SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* renGrp,
	SGObject::UpdateCompGroup<Ov_Sprite::SpriteMap>* sprMap, SGObject::UpdateCompGroup<Ov_Sprite::UpdateAnimationRunning>* runUp, SGRender::Renderer* sprtRen)
{
	std::shared_ptr<Ov_Sprite::RunSprite> sprite(new Ov_Sprite::RunSprite(data, true));
	sprite->setSprite(map.uvTile(data.texture.textureX, data.texture.textureY));
	//Add render group components
	renGrp->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, sprtRen);
	//Add sprite map components
	sprMap->addComponent(&sprite->m_UpdateComps, &sprite->m_Sprite, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &map, data.texture);
	//Add running map components
	runUp->addComponent(&sprite->m_UpdateComps, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &sprite->m_Direction, &sprite->m_Walking, &sprite->m_Running);
	return sprite;
}

std::shared_ptr<Ov_Sprite::RunSprite> Ov_ObjCreation::BuildRunningSprite(Ov_Sprite::SpriteData data, TileMap& map, SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* renGrp,
	SGObject::UpdateCompGroup<Ov_Sprite::SpriteMap>* sprMap, SGRender::Renderer* sprtRen)
{
	std::shared_ptr<Ov_Sprite::RunSprite> sprite(new Ov_Sprite::RunSprite(data, true));
	sprite->setSprite(map.uvTile(data.texture.textureX, data.texture.textureY));
	//Add render group components
	renGrp->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, sprtRen);
	//Add sprite map components
	sprMap->addComponent(&sprite->m_UpdateComps, &sprite->m_Sprite, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &map, data.texture);
	return sprite;
}