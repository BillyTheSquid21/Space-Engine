#include "game/states/StateRenderers.h"

void OverworldRenderer::initialiseRenderer(unsigned int width, unsigned int height)
{
	//Renderer setup
	camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

	//World Renderer
	worldRenderer.setLayout<float>(3, 2);
	worldRenderer.setDrawingMode(GL_TRIANGLES);
	worldRenderer.generate((float)width, (float)height, &camera);

	//Sprite Renderer
	spriteRenderer.setLayout<float>(3, 2);
	spriteRenderer.setDrawingMode(GL_TRIANGLES);
	spriteRenderer.generate((float)width, (float)height, &camera);

	//Model Renderer
	modelRenderer.setLayout<float>(3, 2);
	modelRenderer.setDrawingMode(GL_TRIANGLES);
	modelRenderer.generate((float)width, (float)height, &camera);

	//Camera
	camera.moveUp(World::TILE_SIZE * 5);
	camera.panYDegrees(45.0f);
}

void OverworldRenderer::loadRendererData()
{
	//Shader
	shader.create("res/shaders/Default_T_Shader.glsl");

	//Load world texture
	worldTexture.loadTexture("res/textures/OW.png");
	worldTexture.generateTexture(WORLD_SLOT);
	worldTexture.bind();
	worldTexture.clearBuffer();

	//Load sprite textures
	spriteTexture.loadTexture("res/textures/Sprite.png");
	spriteTexture.generateTexture(SPRITE_SLOT);
	spriteTexture.bind();
	spriteTexture.clearBuffer();
}

void OverworldRenderer::generateAtlas()
{
	modelAtlas.generateAtlas();
	modelAtlas.generateTexture(ATLAS_SLOT);
	modelAtlas.bind();
}

void OverworldRenderer::purgeData() 
{ 
	worldTexture.unbind(); 
}

void OverworldRenderer::bufferRenderData()
{
	spriteRenderer.bufferVideoData();
	modelRenderer.bufferVideoData();
	worldRenderer.bufferVideoData();
}

void OverworldRenderer::draw()
{
	//Send cam uniforms
	camera.sendCameraUniforms(shader);

	//Draw
	shader.setUniform("u_Texture", SPRITE_SLOT);
	spriteRenderer.drawPrimitives(shader);
	shader.setUniform("u_Texture", ATLAS_SLOT);
	modelRenderer.drawPrimitives(shader);
	shader.setUniform("u_Texture", WORLD_SLOT);
	worldRenderer.drawPrimitives(shader);
}