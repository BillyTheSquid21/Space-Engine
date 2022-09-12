#include "game/objects/Tree.h"

void TreeRenderComponent::addTree(Struct2f levelOrigin, World::Tile tile, World::WorldHeight level, TileUV uv1, TileUV uv2)
{
	using namespace Geometry; using namespace SGRender;
	if (m_Trees.quadCount >= m_Trees.quads.size())
	{
		EngineLog("Try reserving more space, tree count out of range!");
		return;
	}

	//Dimensions
	const unsigned int TREE_HEIGHT = World::TILE_SIZE * 2; //Trees are treated as two rectangles to allow borders
	float xPos1 = levelOrigin.a + tile.x * (float)World::TILE_SIZE - (float)World::TILE_SIZE / 2.0f;
	float xPos2 = levelOrigin.a + tile.x * (float)World::TILE_SIZE + (float)World::TILE_SIZE / 2.0f;
	float zPos =  -1*levelOrigin.b + tile.z * (float)World::TILE_SIZE + (float)World::TILE_SIZE / 2.0f;
	float yPos = ((int)level / sqrt(2)) * World::TILE_SIZE;

	//Make Sprite
	Norm_Tex_Quad tree1 = CreateNormalTextureQuad(xPos1, yPos + TREE_HEIGHT, (float)World::TILE_SIZE, TREE_HEIGHT + 10.0f, uv1.u, uv1.v, uv1.width, uv1.height);
	Norm_Tex_Quad tree2 = CreateNormalTextureQuad(xPos2, yPos + TREE_HEIGHT, (float)World::TILE_SIZE, TREE_HEIGHT + 10.0f, uv2.u, uv2.v, uv2.width, uv2.height);
	Norm_Tex_Quad tree3 = CreateNormalTextureQuad(xPos1, yPos + TREE_HEIGHT, (float)World::TILE_SIZE, TREE_HEIGHT + 10.0f, uv1.u, uv1.v, uv1.width, uv1.height);
	Norm_Tex_Quad tree4 = CreateNormalTextureQuad(xPos2, yPos + TREE_HEIGHT, (float)World::TILE_SIZE, TREE_HEIGHT + 10.0f, uv2.u, uv2.v, uv2.width, uv2.height);

	//Position Sprite
	Translate<NTVertex>((NTVertex*)&tree1, 1.0f, 0.0f, -zPos, Shape::QUAD);
	Translate<NTVertex>((NTVertex*)&tree2, 1.0f, 0.0f, -zPos, Shape::QUAD);
	
	AxialRotate<NTVertex>((NTVertex*)&tree3, { xPos1 + World::TILE_SIZE / 2, yPos, 0.0f }, 90.0f, Shape::QUAD, Axis::Y);
	Translate<NTVertex>((NTVertex*)&tree3, 1.0f + World::TILE_SIZE / 2, 0.0f, -zPos + (float)World::TILE_SIZE / 2.0f, Shape::QUAD);
	AxialRotate<NTVertex>((NTVertex*)&tree4, { xPos2 - World::TILE_SIZE / 2, yPos, 0.0f }, 90.0f, Shape::QUAD, Axis::Y);
	Translate<NTVertex>((NTVertex*)&tree4, 1.0f + World::TILE_SIZE / 2, 0.0f, -zPos + (float)World::TILE_SIZE / 2.0f, Shape::QUAD);

	//Gen normals
	CalculateQuadNormals((NTVertex*)&tree1);
	CalculateQuadNormals((NTVertex*)&tree2);
	CalculateQuadNormals((NTVertex*)&tree3);
	CalculateQuadNormals((NTVertex*)&tree4);
	
	m_Trees.quads[m_Trees.quadCount] = tree1;
	m_Trees.quadCount++;
	m_Trees.quads[m_Trees.quadCount] = tree2;
	m_Trees.quadCount++;
	m_Trees.quads[m_Trees.quadCount] = tree3;
	m_Trees.quadCount++;
	m_Trees.quads[m_Trees.quadCount] = tree4;
	m_Trees.quadCount++;
}

void TreeRenderComponent::generateIndices()
{
	GenerateQuadArrayIndices(m_Trees);
}