#include "game/objects/OverworldSprite.h"

//components
void SpriteRender::render()
{
	if (m_Sprite != nullptr && m_Renderer != nullptr) {
		m_Renderer->commit((TextureVertex*)m_Sprite, GetFloatCount<TextureVertex>(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT);
	}
}

void TilePosition::update(double deltaTime)
{
	//Works out current tile
	Struct2f origin = World::Level::queryOrigin(*m_CurrentLevel);
	float deltaX = *m_XPos - origin.a;
	float deltaZ = *m_ZPos - origin.b;
	*m_TileX = (unsigned int)(deltaX / World::TILE_SIZE);
	*m_TileZ = (unsigned int)(-deltaZ / World::TILE_SIZE);
}

void SpriteMap::update(double deltaTime)
{
	if (m_LastOffX != *m_OffsetX || m_LastOffY != *m_OffsetY)
	{
		//Update sprite map
		UVData data = m_Map->uvTile(m_TextureOrigin.textureX + *m_OffsetX, m_TextureOrigin.textureY + *m_OffsetY);
		SetQuadUV((TextureVertex*)m_Sprite, data.u, data.v, data.width, data.height);
		m_LastOffX = *m_OffsetX; m_LastOffY = *m_OffsetY;
	}
}

void UpdateAnimationFacing::update(double deltaTime)
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

void UpdateAnimationWalking::update(double deltaTime)
{
	UpdateAnimationFacing::update(deltaTime);

	if (*m_Walking)
	{
		if (m_Timer <= 0.25f)
		{
			*m_OffsetX = 1;
		}
		else if (m_Timer > 0.25f && m_Timer <= 0.5f)
		{
			*m_OffsetX = 0;
		}
		else if (m_Timer > 0.5f && m_Timer <= 0.75f)
		{
			*m_OffsetX = 2;
		}
		else if (m_Timer > 0.75f && m_Timer <= 1.0f)
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

void UpdateAnimationRunning::update(double deltaTime)
{
	UpdateAnimationWalking::update(deltaTime);
	if (*m_Running)
	{
		if (m_Timer <= 0.125f)
		{
			*m_OffsetX = 4;
		}
		else if (m_Timer > 0.125f && m_Timer <= 0.25f)
		{
			*m_OffsetX = 3;
		}
		else if (m_Timer > 0.25f && m_Timer <= 0.375f)
		{
			*m_OffsetX = 5;
		}
		else if (m_Timer > 0.375f && m_Timer <= 0.5f)
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
OvSpr_Sprite::OvSpr_Sprite(OvSpr_SpriteData data)
{
	using namespace World;
	//Set
	m_TileX = data.tile.x; m_TileZ = data.tile.z; m_CurrentLevel = data.levelID;
	//X Y Z
	//Get origin
	Struct2f origin = World::Level::queryOrigin(data.levelID);
	float x = origin.a + (m_TileX * TILE_SIZE); float y = ((float)data.height / sqrt(2)) * TILE_SIZE;
	float z = origin.b -(m_TileZ * TILE_SIZE);
	//Set pos - x in tile middle
	m_XPos = x + TILE_SIZE / 2; m_YPos = y; m_ZPos = z - TILE_SIZE / 2;
	//Make Sprite
	m_Sprite = CreateTextureQuad(x, y + TILE_SIZE, TILE_SIZE, TILE_SIZE, 0.0f, 0.0f, 0.05f, 0.1f);
	//Position Sprite
	RotateShape<TextureVertex>((TextureVertex*)&m_Sprite, { x + TILE_SIZE / 2, y, 0.0f }, -45.0f, Shape::QUAD, Axis::X);
	TranslateShape<TextureVertex>((TextureVertex*)&m_Sprite, 0.0f, 0.0f, m_ZPos, Shape::QUAD);

	//Make current tile blocked
	std::vector<MovementPermissions>* perm = Level::queryPermissions(m_CurrentLevel);
	World::LevelDimensions dim = Level::queryDimensions(m_CurrentLevel);
	unsigned int tileLocation = m_TileX * dim.levelH + m_TileZ;
	perm->at(tileLocation) = MovementPermissions::SPRITE_BLOCKING;
}

void OvSpr_Sprite::setSprite(UVData data)
{
	SetQuadUV((TextureVertex*)&m_Sprite, data.u, data.v, data.width, data.height);
}

//Translation
void Ov_Translation::Walk(World::Direction* direction, float* x, float* z, TextureQuad* sprite, double deltaTime, double* walkTimer)
{
	using namespace World;
	switch (*direction)
	{
	case Direction::EAST:
		*x += deltaTime * TILE_SIZE * WALK_SPEED;
		TranslateShape<TextureVertex>(sprite, deltaTime * TILE_SIZE * WALK_SPEED, 0.0f, 0.0f, Shape::QUAD);
		break;
	case Direction::WEST:
		*x += deltaTime * -TILE_SIZE * WALK_SPEED;
		TranslateShape<TextureVertex>(sprite, deltaTime * -TILE_SIZE * WALK_SPEED, 0.0f, 0.0f, Shape::QUAD);
		break;
	case Direction::NORTH:
		*z += deltaTime * -TILE_SIZE * WALK_SPEED;
		TranslateShape<TextureVertex>(sprite, 0.0f, 0.0f, deltaTime * -TILE_SIZE * WALK_SPEED, Shape::QUAD);
		break;
	case Direction::SOUTH:
		*z += deltaTime * TILE_SIZE * WALK_SPEED;
		TranslateShape<TextureVertex>(sprite, 0.0f, 0.0f, deltaTime * TILE_SIZE * WALK_SPEED, Shape::QUAD);
		break;
	default:
		break;
	}

	*walkTimer += deltaTime;
}

void Ov_Translation::Run(World::Direction* direction, float* x, float* z, TextureQuad* sprite, double deltaTime, double* walkTimer)
{
	using namespace World;
	switch (*direction)
	{
	case Direction::EAST:
		*x += deltaTime * TILE_SIZE * RUN_SPEED;
		TranslateShape<TextureVertex>(sprite, deltaTime * TILE_SIZE * RUN_SPEED, 0.0f, 0.0f, Shape::QUAD);
		break;
	case Direction::WEST:
		*x += deltaTime * -TILE_SIZE * RUN_SPEED;
		TranslateShape<TextureVertex>(sprite, deltaTime * -TILE_SIZE * RUN_SPEED, 0.0f, 0.0f, Shape::QUAD);
		break;
	case Direction::NORTH:
		*z += deltaTime * -TILE_SIZE * RUN_SPEED;
		TranslateShape<TextureVertex>(sprite, 0.0f, 0.0f, deltaTime * -TILE_SIZE * RUN_SPEED, Shape::QUAD);
		break;
	case Direction::SOUTH:
		*z += deltaTime * TILE_SIZE * RUN_SPEED;
		TranslateShape<TextureVertex>(sprite, 0.0f, 0.0f, deltaTime * TILE_SIZE * RUN_SPEED, Shape::QUAD);
		break;
	default:
		break;
	}

	*walkTimer += deltaTime;
}

void Ov_Translation::CentreOnTile(World::LevelID currentLevel, World::WorldLevel worldLevel, float* x, float* y, float* z, unsigned int tileX, unsigned int tileZ, TextureQuad* sprite)
{
	//Centre on x and y
	Struct2f origin = World::Level::queryOrigin(currentLevel);
	float expectedX = (float)(World::TILE_SIZE * tileX) + origin.a + World::TILE_SIZE / 2;
	float expectedZ = (float)(-World::TILE_SIZE * tileZ) + origin.b - World::TILE_SIZE / 2;
	float expectedY = ((int)worldLevel / sqrt(2)) * World::TILE_SIZE;
	float deltaX = *x - expectedX;
	float deltaZ = *z - expectedZ;
	float deltaY = *y - expectedY;
	TranslateShape<TextureVertex>(sprite, -deltaX, -deltaY, -deltaZ, Shape::QUAD);
	*x = expectedX;
	*z = expectedZ;
	*y = expectedY;
}

void Ov_Translation::CentreOnTile(World::LevelID currentLevel, World::WorldLevel worldLevel, float* x, float* y, float* z, unsigned int tileX, unsigned int tileZ, TextureQuad* sprite, bool onSlope)
{
	//Centre on x and y
	Struct2f origin = World::Level::queryOrigin(currentLevel);
	float expectedX = (float)(World::TILE_SIZE * tileX) + origin.a + World::TILE_SIZE / 2;
	float expectedZ = (float)(-World::TILE_SIZE * tileZ) + origin.b - World::TILE_SIZE / 2;
	float expectedY = ((int)worldLevel / sqrt(2)) * World::TILE_SIZE;
	if (onSlope)
	{
		expectedY = ((int)worldLevel / (sqrt(2))) * World::TILE_SIZE - ((int)1 / (2 * (sqrt(2)))) * World::TILE_SIZE;
	}
	float deltaX = *x - expectedX;
	float deltaZ = *z - expectedZ;
	float deltaY = *y - expectedY;
	TranslateShape<TextureVertex>(sprite, -deltaX, -deltaY, -deltaZ, Shape::QUAD);
	*x = expectedX;
	*z = expectedZ;
	*y = expectedY;
}

void Ov_Translation::AscendSlope(float* y, TextureQuad* sprite, double deltaTime, bool running)
{
	if (!running)
	{
		*y += (World::WALK_SPEED / sqrt(2))* World::TILE_SIZE * deltaTime;
		TranslateShape<TextureVertex>(sprite, 0.0f, (World::WALK_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime, 0.0f, Shape::QUAD);
	}
	else
	{
		*y += (World::RUN_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime;
		TranslateShape<TextureVertex>(sprite, 0.0f, (World::RUN_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime, 0.0f, Shape::QUAD);
	}
}

void Ov_Translation::AscendSlope(float* y, TextureQuad* sprite, double deltaTime, bool running, double timer, bool verticalFirst
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

void Ov_Translation::DescendSlope(float* y, TextureQuad* sprite, double deltaTime, bool running)
{
	if (!running)
	{
		*y += (-World::WALK_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime;
		TranslateShape<TextureVertex>(sprite, 0.0f, (-World::WALK_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime, 0.0f, Shape::QUAD);
	}
	else
	{
		*y += (-World::RUN_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime;
		TranslateShape<TextureVertex>(sprite, 0.0f, (-World::RUN_SPEED / sqrt(2)) * World::TILE_SIZE * deltaTime, 0.0f, Shape::QUAD);
	}
}

void Ov_Translation::DescendSlope(float* y, TextureQuad* sprite, double deltaTime, bool running, double timer, bool verticalFirst)
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


std::shared_ptr<OvSpr_Sprite> Ov_ObjCreation::BuildSprite(OvSpr_SpriteData data, TileMap& map, RenderComponentGroup<SpriteRender>* renGrp, 
	Render::Renderer<TextureVertex>* sprtRen)
{
	std::shared_ptr<OvSpr_Sprite> sprite(new OvSpr_Sprite(data));
	sprite->setSprite(map.uvTile(data.texture.textureX, data.texture.textureY));
	//Add render group components
	renGrp->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, sprtRen);
	return sprite;
}

std::shared_ptr<OvSpr_DirectionalSprite> Ov_ObjCreation::BuildDirectionalSprite(OvSpr_SpriteData data, TileMap& map, RenderComponentGroup<SpriteRender>* renGrp,
	UpdateComponentGroup<SpriteMap>* sprMap, UpdateComponentGroup<UpdateAnimationFacing>* faceUp, Render::Renderer<TextureVertex>* sprtRen)
{
	std::shared_ptr<OvSpr_DirectionalSprite> sprite(new OvSpr_DirectionalSprite(data));
	sprite->setSprite(map.uvTile(data.texture.textureX, data.texture.textureY));
	//Add render group components
	renGrp->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, sprtRen);
	//Add sprite map components
	sprMap->addComponent(&sprite->m_UpdateComps, &sprite->m_Sprite, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &map, data.texture);
	//Add facing map components
	faceUp->addComponent(&sprite->m_UpdateComps, &sprite->m_AnimationOffsetY, &sprite->m_Direction);
	return sprite;
}

std::shared_ptr<OvSpr_WalkingSprite> Ov_ObjCreation::BuildWalkingSprite(OvSpr_SpriteData data, TileMap& map, RenderComponentGroup<SpriteRender>* renGrp, 
	UpdateComponentGroup<SpriteMap>* sprMap, UpdateComponentGroup<UpdateAnimationWalking>* walkUp, Render::Renderer<TextureVertex>* sprtRen)
{
	std::shared_ptr<OvSpr_WalkingSprite> sprite(new OvSpr_WalkingSprite(data));
	sprite->setSprite(map.uvTile(data.texture.textureX, data.texture.textureY));
	//Add render group components
	renGrp->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, sprtRen);
	//Add sprite map components
	sprMap->addComponent(&sprite->m_UpdateComps, &sprite->m_Sprite, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &map, data.texture);
	//Add walking map components
	walkUp->addComponent(&sprite->m_UpdateComps, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &sprite->m_Direction, &sprite->m_Walking);
	return sprite;
}

std::shared_ptr<OvSpr_RunningSprite> Ov_ObjCreation::BuildRunningSprite(OvSpr_SpriteData data, TileMap& map, RenderComponentGroup<SpriteRender>* renGrp, 
	UpdateComponentGroup<SpriteMap>* sprMap, UpdateComponentGroup<UpdateAnimationRunning>* runUp, Render::Renderer<TextureVertex>* sprtRen)
{
	std::shared_ptr<OvSpr_RunningSprite> sprite(new OvSpr_RunningSprite(data));
	sprite->setSprite(map.uvTile(data.texture.textureX, data.texture.textureY));
	//Add render group components
	renGrp->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, sprtRen);
	//Add sprite map components
	sprMap->addComponent(&sprite->m_UpdateComps, &sprite->m_Sprite, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &map, data.texture);
	//Add running map components
	runUp->addComponent(&sprite->m_UpdateComps, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &sprite->m_Direction, &sprite->m_Walking, &sprite->m_Running);
	return sprite;
}