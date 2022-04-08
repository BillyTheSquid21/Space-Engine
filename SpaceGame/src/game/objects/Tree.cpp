#include "game/objects/Tree.h"

void TreeRenderComponent::addTree(Struct2f levelOrigin, World::TileLoc tile, World::WorldLevel level, UVData uv1, UVData uv2)
{
	if (m_QuadCount >= m_Trees.size())
	{
		EngineLog("Try reserving more space, tree count out of range!");
		return;
	}

	//Dimensions
	const unsigned int TREE_HEIGHT = World::TILE_SIZE * 2; //Trees are treated as two rectangles to allow borders
	float xPos1 = tile.x * (float)World::TILE_SIZE - (float)World::TILE_SIZE / 2.0f;
	float xPos2 = tile.x * (float)World::TILE_SIZE + (float)World::TILE_SIZE / 2.0f;
	float zPos = tile.z * (float)World::TILE_SIZE + (float)World::TILE_SIZE / 3.0f;
	float yPos = ((int)level / sqrt(2)) * World::TILE_SIZE;

	//Make Sprite
	TextureQuad tree1 = CreateTextureQuad(xPos1, yPos + TREE_HEIGHT - 5.0f, (float)World::TILE_SIZE, TREE_HEIGHT, uv1.u, uv1.v, uv1.width, uv1.height);
	TextureQuad tree2 = CreateTextureQuad(xPos2, yPos + TREE_HEIGHT - 5.0f, (float)World::TILE_SIZE, TREE_HEIGHT, uv2.u, uv2.v, uv2.width, uv2.height);
	
	//Position Sprite
	RotateShape<TextureVertex>((TextureVertex*)&tree1, { xPos1 + World::TILE_SIZE / 2, yPos, 0.0f }, -45.0f, Shape::QUAD, Axis::X);
	TranslateShape<TextureVertex>((TextureVertex*)&tree1, 0.0f, 0.0f, -zPos, Shape::QUAD);
	RotateShape<TextureVertex>((TextureVertex*)&tree2, { xPos2 + World::TILE_SIZE / 2, yPos, 0.0f }, -45.0f, Shape::QUAD, Axis::X);
	TranslateShape<TextureVertex>((TextureVertex*)&tree2, 0.0f, 0.0f, -zPos, Shape::QUAD);
	m_Trees[m_QuadCount] = tree1;
	m_QuadCount++;
	m_Trees[m_QuadCount] = tree2;
	m_QuadCount++;
}

void TreeRenderComponent::generateIndices()
{
	//Buffer indices to minimise counts of data sent to render queue
	//Find total ints needed
	unsigned int quadIntCount = m_QuadCount * Primitive::Q_IND_COUNT;

	//Resize vector to be able to fit and init index
	m_Indices.resize(quadIntCount);
	unsigned int indicesIndex = 0;

	unsigned int lastLargest = -1;
	unsigned int indicesTemp[6]{ 0,1,2,0,2,3 };
	for (unsigned int i = 0; i < m_QuadCount; i++) {
		//Increment all by last largest - set temp to base
		std::copy(&Primitive::Q_IND[0], &Primitive::Q_IND[Primitive::Q_IND_COUNT], &indicesTemp[0]);
		for (int j = 0; j < Primitive::Q_IND_COUNT; j++) {
			indicesTemp[j] += lastLargest + 1;
		}
		//Set last largest
		lastLargest = indicesTemp[Primitive::Q_IND_COUNT - 1];
		//Copy into vector
		std::copy(&indicesTemp[0], &indicesTemp[Primitive::Q_IND_COUNT], m_Indices.begin() + indicesIndex);
		//Increment index
		indicesIndex += Primitive::Q_IND_COUNT;
	}
}