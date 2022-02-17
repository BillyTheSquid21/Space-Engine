#include "game/objects/OWObjects.h"

//components
WorldLocation::WorldLocation(float& x, float& z, unsigned int& tilePointX, unsigned int& tilePointZ)
	:	m_XRef(x), m_ZRef(z), m_CurrentTileX(tilePointX), m_CurrentTileZ(tilePointZ){}

void WorldLocation::setTileStart(float& yPos, World::WorldLevel height, unsigned int xTile, unsigned int zTile) {
	m_CurrentTileX = xTile; m_CurrentTileZ = zTile;

	//Locate where in space sprite is
	m_XRef = (World::TILE_SIZE * xTile) + m_LevelOriginX;
	m_ZRef = (World::TILE_SIZE * zTile) + m_LevelOriginZ;
	//Height
	yPos = (((float)height / sqrt(2)) * World::TILE_SIZE) + World::TILE_SIZE;
}

void WorldLocation::function(double deltaTime, double time)	{
	//Check location in world, update tile location if has changed
	float locationX = m_XRef - m_LevelOriginX;
	float locationZ = m_ZRef - m_LevelOriginZ;

	int currentTileX = (int)locationX / World::TILE_SIZE;
	int currentTileZ = (int)locationZ / World::TILE_SIZE;
	if (currentTileX < 0 || currentTileZ < 0) {
		//Outside of bounds, ignore as when level changes will be automatically changed
		return;
	}
	if (m_CurrentTileX != (int)currentTileX || m_CurrentTileZ != (int)currentTileZ) {
		m_CurrentTileX = (unsigned int)currentTileX;	m_CurrentTileZ = (unsigned int)currentTileZ;
		EngineLog(m_CurrentTileX, m_CurrentTileZ);
	}
	
}

PlayerMove::PlayerMove(bool& up, bool& down, bool& left, bool& right, float& x, float& y, float& z, TextureQuad& sprite)
	:	m_HeldUp(up), m_HeldDown(down), m_HeldLeft(left), m_HeldRight(right), m_XRef(x), m_YRef(y), m_ZRef(z), m_Sprite(sprite){}

void PlayerMove::function(double deltaTime, double time) {
	//This will not be how motion works, it is for debugging
	if (m_HeldRight && !m_Walking) {
		if (checkPermissions()) {
			m_Walking = true;
			m_Direction = World::Direction::EAST;
		}
	}
	else if (m_HeldLeft && !m_Walking) {
		if (checkPermissions()) {
			m_Walking = true;
			m_Direction = World::Direction::WEST;
		}
	}
	else if (m_HeldUp && !m_Walking) {
		if (checkPermissions()) {
			m_Walking = true;
			m_Direction = World::Direction::NORTH;
		}
	}
	else if (m_HeldDown && !m_Walking) {
		if (checkPermissions()) {
			m_Walking = true;
			m_Direction = World::Direction::SOUTH;
		}
	}

	//Don't execute if not walking
	if (!m_Walking) {
		return;
	}

	//Move
	switch (m_Direction) {
	case World::Direction::EAST:
		m_Timer += deltaTime;
		m_XRef += World::TILE_SIZE * deltaTime;
		TranslateShape<TextureVertex>(&m_Sprite, World::TILE_SIZE * deltaTime, 0.0f, 0.0f, Shape::QUAD);
		break;
	case World::Direction::WEST:
		m_Timer += deltaTime;
		m_XRef += World::TILE_SIZE * -deltaTime;
		TranslateShape<TextureVertex>(&m_Sprite, World::TILE_SIZE * -deltaTime, 0.0f, 0.0f, Shape::QUAD);
		break;
	case World::Direction::NORTH:
		m_Timer += deltaTime;
		m_ZRef += World::TILE_SIZE * -deltaTime;
		TranslateShape<TextureVertex>(&m_Sprite, 0.0f, 0.0f, World::TILE_SIZE * -deltaTime, Shape::QUAD);
		break;
	case World::Direction::SOUTH:
		m_Timer += deltaTime;
		m_ZRef += World::TILE_SIZE * deltaTime;
		TranslateShape<TextureVertex>(&m_Sprite, 0.0f, 0.0f, World::TILE_SIZE * deltaTime, Shape::QUAD);
		break;
	default:
		EngineLog("Invalid direction");
		break;
	}

	//When walk over - TODO make tell difference if negative
	if (m_Timer >= 1.0) {
		float xOvershoot = m_XRef - (float)roundDownMultiple(m_XRef, World::TILE_SIZE);
		float zOvershoot = m_ZRef - (float)roundDownMultiple(m_ZRef, World::TILE_SIZE);
		TranslateShape<TextureVertex>(&m_Sprite, -xOvershoot, 0.0f, 0.0f, Shape::QUAD);
		m_XRef = (float)roundDownMultiple(m_XRef, World::TILE_SIZE);
		m_Timer = 0.0;
		m_Walking = false;
	}
}

//objects
void Sprite::generate(float xPos, float zPos, World::LevelID level, unsigned int components) {
	//Set position
	m_XPos = xPos;
	m_ZPos = zPos;
	m_LevelID = level;
	//Init component array
	//Create and add component
	WorldLocation* location = new WorldLocation(m_XPos, m_ZPos, m_CurrentTileX, m_CurrentTileZ);
	location->setLevelOrigin({ 0.0f, 0.0f });
	location->setTileStart(m_YPos, World::WorldLevel::F0, 1, 4);
	Component* locationBase = (Component*)location;
	locationBase->setID(m_Components.size());
	m_Components.push_back(locationBase);

	//Generate quad
	m_Sprite = CreateTextureQuad(m_XPos, m_YPos, World::TILE_SIZE, World::TILE_SIZE, 0.0f, 0.0f, 1.0f, 1.0f);
	TranslateShape<TextureVertex>((TextureVertex*)&m_Sprite, 0.0f, 0.0f, -m_ZPos, Shape::QUAD);
}

void Sprite::render() {
	m_Renderer->commit((TextureVertex*)&m_Sprite, GetFloatCount<TextureVertex>(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT);
}

void Sprite::update(double deltaTime, double time) {
	for (int i = 0; i < m_Components.size(); i++) { m_Components[i]->function(deltaTime, time); }
}

//Player
void Player::generate(float xPos, float yPos, bool& up, bool& down, bool& left, bool& right, World::LevelID level, unsigned int components) {
	Sprite::generate(xPos, yPos, level, components);

	PlayerMove* move = new PlayerMove(up, down, left, right, m_XPos, m_YPos, m_ZPos, m_Sprite);
	Component* moveBase = (Component*)move;
	moveBase->setID(m_Components.size());
	m_Components.push_back(moveBase);
}