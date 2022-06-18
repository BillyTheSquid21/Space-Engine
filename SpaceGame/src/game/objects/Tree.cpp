#include "game/objects/Tree.h"

void TreeRenderComponent::addTree(Struct2f levelOrigin, World::TileLoc tile, World::WorldHeight level, TileUV uv1, TileUV uv2)
{
	if (m_Trees.quadCount >= m_Trees.quads.size())
	{
		EngineLog("Try reserving more space, tree count out of range!");
		return;
	}

	//Dimensions
	const unsigned int TREE_HEIGHT = World::TILE_SIZE * 2; //Trees are treated as two rectangles to allow borders
	float xPos1 = levelOrigin.a + tile.x * (float)World::TILE_SIZE - (float)World::TILE_SIZE / 2.0f;
	float xPos2 = levelOrigin.a + tile.x * (float)World::TILE_SIZE + (float)World::TILE_SIZE / 2.0f;
	float zPos =  -1*levelOrigin.b + tile.z * (float)World::TILE_SIZE + (float)World::TILE_SIZE / 3.0f;
	float yPos = ((int)level / sqrt(2)) * World::TILE_SIZE;

	//Make Sprite
	Norm_Tex_Quad tree1 = CreateNormalTextureQuad(xPos1, yPos + TREE_HEIGHT - 5.0f, (float)World::TILE_SIZE, TREE_HEIGHT, uv1.u, uv1.v, uv1.width, uv1.height);
	Norm_Tex_Quad tree2 = CreateNormalTextureQuad(xPos2, yPos + TREE_HEIGHT - 5.0f, (float)World::TILE_SIZE, TREE_HEIGHT, uv2.u, uv2.v, uv2.width, uv2.height);
	
	//Position Sprite
	RotateShape<NormalTextureVertex>((NormalTextureVertex*)&tree1, { xPos1 + World::TILE_SIZE / 2, yPos, 0.0f }, -45.0f, Shape::QUAD, Axis::X);
	TranslateShape<NormalTextureVertex>((NormalTextureVertex*)&tree1, 0.0f, 0.0f, -zPos, Shape::QUAD);
	RotateShape<NormalTextureVertex>((NormalTextureVertex*)&tree2, { xPos2 + World::TILE_SIZE / 2, yPos, 0.0f }, -45.0f, Shape::QUAD, Axis::X);
	TranslateShape<NormalTextureVertex>((NormalTextureVertex*)&tree2, 0.0f, 0.0f, -zPos, Shape::QUAD);

	//Gen normals
	CalculateQuadNormals((NormalTextureVertex*)&tree1);
	CalculateQuadNormals((NormalTextureVertex*)&tree2);
	
	m_Trees.quads[m_Trees.quadCount] = tree1;
	m_Trees.quadCount++;
	m_Trees.quads[m_Trees.quadCount] = tree2;
	m_Trees.quadCount++;
}

void TreeRenderComponent::generateIndices()
{
	GenerateQuadArrayIndices(m_Trees);
}