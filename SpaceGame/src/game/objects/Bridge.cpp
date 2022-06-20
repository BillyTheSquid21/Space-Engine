#include "game/objects/Bridge.h"

BridgeRenderComponent::BridgeRenderComponent(Struct2f pos, World::WorldHeight height, unsigned int width, unsigned int length, TileUV data1, TileUV data2, Render::Renderer<NormalTextureVertex>* ren, bool horizontal)
{
	using World::TILE_SIZE;
	constexpr float bridgeOffset = 9.0f / 32.0f;

	m_Tiles.quadCount = width * length * 2;
	m_Tiles.quads.resize(m_Tiles.quadCount);
	m_Renderer = ren;

	//Create quads for bridge - pos is at top left
	unsigned int index = 0;

	//Height
	float yPos = ((float)height / sqrt(2)) * TILE_SIZE;

	bool alternate = false;
	for (int z = 0; z < length; z++)
	{
		for (int x = 0; x < width; x++)
		{
			//Positions
			float xPos = pos.a + x * TILE_SIZE;
			float zPos = pos.b + z * TILE_SIZE;
			
			//Make Sprite
			Norm_Tex_Quad tile;
			Norm_Tex_Quad tileSide;
			//Right
			if (alternate) 
			{
				tile = CreateNormalTextureQuad(xPos, yPos + TILE_SIZE, TILE_SIZE, TILE_SIZE, data1.u, data1.v, data1.width - bridgeOffset * data1.width, data1.height);
				tileSide = CreateNormalTextureQuad(xPos, yPos + TILE_SIZE, 9.0f, TILE_SIZE, data1.u + data1.width - bridgeOffset * data1.width, data1.v, bridgeOffset * data1.width, data1.height);
				AxialRotate<NormalTextureVertex>((NormalTextureVertex*)&tileSide, { xPos, yPos, 0.0f }, -90.0f, Shape::QUAD, Axis::Z);
				AxialRotate<NormalTextureVertex>((NormalTextureVertex*)&tileSide, { xPos, yPos, 0.0f }, 90.0f, Shape::QUAD, Axis::Y);
				Translate<NormalTextureVertex>((NormalTextureVertex*)&tileSide, TILE_SIZE, 0.0f, zPos, Shape::QUAD);
				if (!horizontal)
				{
					alternate = false;
				}
				
			}
			//Left
			else if (!alternate)
			{
				tile = CreateNormalTextureQuad(xPos, yPos + TILE_SIZE, TILE_SIZE, TILE_SIZE, data2.u + bridgeOffset * data1.width, data2.v, data2.width - bridgeOffset * data1.width, data2.height);
				tileSide = CreateNormalTextureQuad(xPos, yPos + TILE_SIZE, 9.0f, TILE_SIZE, data2.u, data2.v, bridgeOffset * data2.width, data2.height);
				AxialRotate<NormalTextureVertex>((NormalTextureVertex*)&tileSide, { xPos, yPos, 0.0f }, 90.0f, Shape::QUAD, Axis::Z); 
				AxialRotate<NormalTextureVertex>((NormalTextureVertex*)&tileSide, { xPos-32.0f, yPos, 0.0f }, 90.0f, Shape::QUAD, Axis::Y);
				Translate<NormalTextureVertex>((NormalTextureVertex*)&tileSide, TILE_SIZE, -9.0f, zPos, Shape::QUAD);
				if (!horizontal)
				{
					alternate = true;
				}
			}
			
			//Position Sprite
			AxialRotate<NormalTextureVertex>((NormalTextureVertex*)&tile, { x + TILE_SIZE / 2, yPos, 0.0f }, -90.0f, Shape::QUAD, Axis::X);
			Translate<NormalTextureVertex>((NormalTextureVertex*)&tile, 0.0f, 0.0f, zPos, Shape::QUAD); 
			
			//If horizontal, rotate about centre
			if (horizontal)
			{
				AxialRotate<NormalTextureVertex>((NormalTextureVertex*)&tile, { xPos + TILE_SIZE/2 , yPos, zPos-TILE_SIZE/2 }, -90.0f, Shape::QUAD, Axis::Y);
				AxialRotate<NormalTextureVertex>((NormalTextureVertex*)&tileSide, { xPos + TILE_SIZE / 2 , yPos, zPos - TILE_SIZE / 2 }, -90.0f, Shape::QUAD, Axis::Y);
			}

			//Gen normals
			CalculateQuadNormals((NormalTextureVertex*)&tile);
			CalculateQuadNormals((NormalTextureVertex*)&tileSide);
			
			m_Tiles.quads[index] = tile;
			index++;
			m_Tiles.quads[index] = tileSide;
			index++;
		}
		if (alternate && horizontal)
		{
			alternate = false;
		}
		else if (!alternate && horizontal)
		{
			alternate = true;
		}
	}
}

void BridgeRenderComponent::generateIndices()
{
	GenerateQuadArrayIndices(m_Tiles);
}