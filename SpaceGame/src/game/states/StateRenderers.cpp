#include "game/states/StateRenderers.h"

void OverworldRenderer::initialiseRenderer(unsigned int width, unsigned int height)
{
	//Renderer setup
	camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

	//World Renderer
	worldRenderer.setLayout<float>(3, 2, 3);
	worldRenderer.setDrawingMode(GL_TRIANGLES);
	worldRenderer.generate((float)width, (float)height, &camera);

	//Sprite Renderer
	spriteRenderer.setLayout<float>(3, 2, 3);
	spriteRenderer.setDrawingMode(GL_TRIANGLES);
	spriteRenderer.generate((float)width, (float)height, &camera);

	//Model Renderer
	modelRenderer.setLayout<float>(3, 2, 3);
	modelRenderer.setDrawingMode(GL_TRIANGLES);
	modelRenderer.generate((float)width, (float)height, &camera);

	//Camera
	camera.moveUp(World::TILE_SIZE * 5);
	camera.panYDegrees(45.0f);
}

void OverworldRenderer::loadRendererData()
{
	//Shader
	shader.create("res/shaders/Lighting_T_Shader.glsl");

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

	//Update player pos
	glm::vec3 lightDir = glm::normalize(m_LightDir);
	shader.setUniform("u_LightScene", m_LightScene);

	//Set universal uniforms
	shader.setUniform("u_AmbLight", 0.5f);
	shader.setUniform("u_LightDir", &lightDir);

	//Sprites
	shader.setUniform("u_Texture", SPRITE_SLOT);
	glm::mat4 SpriteInvTranspModel = glm::mat4(glm::transpose(glm::inverse(spriteRenderer.m_RendererModelMatrix)));
	shader.setUniform("u_InvTranspModel", &SpriteInvTranspModel);
	spriteRenderer.drawPrimitives(shader);

	//Models - Make have per model inv transp later when models are used more
	shader.setUniform("u_Texture", ATLAS_SLOT);
	modelRenderer.drawPrimitives(shader);

	//World
	shader.setUniform("u_Texture", WORLD_SLOT);
	glm::mat4 WorldInvTranspModel = glm::mat4(glm::transpose(glm::inverse(worldRenderer.m_RendererModelMatrix)));
	shader.setUniform("u_InvTranspModel", &WorldInvTranspModel);
	worldRenderer.drawPrimitives(shader);
}