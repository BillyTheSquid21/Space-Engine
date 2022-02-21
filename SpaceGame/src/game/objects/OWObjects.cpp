#include "game/objects/OWObjects.h"

//components
void SpriteRender::render()
{
	m_Renderer->commit((TextureVertex*)&m_Sprite, GetFloatCount<TextureVertex>(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT);
}