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

	//Shadow Map
	shadowMap.init();
	glm::mat4 lightProj = glm::ortho(-1024.0f, 1024.0f, -512.0f, 512.0f, 0.1f, 1000.0f);
	shadowMap.setProjection(lightProj);

	SCREEN_HEIGHT = height; SCREEN_WIDTH = width;
}

void OverworldRenderer::loadRendererData()
{
	//Shader
	shader.create("res/shaders/Lighting_T_Shader.glsl");
	shadows.create("res/shaders/Shadows_Shader.glsl");

	//Load world texture
	worldTexture.loadTexture("res/textures/OW.png");
	worldTexture.generateTexture(TEXTURE_SLOT);
	worldTexture.bind();
	worldTexture.clearBuffer();

	//Load sprite textures
	spriteTexture.loadTexture("res/textures/Sprite.png");
	spriteTexture.generateTexture(TEXTURE_SLOT);
	spriteTexture.bind();
	spriteTexture.clearBuffer();
}

void OverworldRenderer::generateAtlas()
{
	modelAtlas.generateAtlas();
	modelAtlas.generateTexture(TEXTURE_SLOT);
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Send cam uniforms
	shader.bind();
	camera.sendCameraUniforms(shader);

	//Update player pos
	glm::vec3 lightDir = glm::normalize(m_LightDir);

	//Shadow pass
	shadowMap.bindForWriting();
	shadows.bind();

	//matrixes
	glm::mat4 world = worldRenderer.m_RendererModelMatrix;
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightView = glm::translate(lightView, -camera.getPos());
	shadows.setUniform("WVP", shadowMap.calcMVP(world, lightView));
	shadowMap.startCapture();
	worldTexture.bind();
	worldRenderer.drawPrimitives(shadows);
	spriteTexture.bind();
	spriteRenderer.drawPrimitives(shadows);
	shadowMap.endCapture(SCREEN_WIDTH, SCREEN_HEIGHT);
	shadows.unbind();

	// reset viewport
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Lighting pass
	shader.bind();
	shadowMap.bindForReading(SHADOWS_SLOT);

	//Set universal uniforms
	shader.setUniform("u_AmbLight", 0.5f);
	shader.setUniform("u_LightDir", &lightDir);
	shader.setUniform("u_LightMVP", shadowMap.getMVP());
	shader.setUniform("u_ShadowMap", SHADOWS_SLOT);
	shader.setUniform("u_LightsActive", lightScene);
	shader.setUniform("u_Texture", TEXTURE_SLOT);

	//Sprites
	spriteTexture.bind();
	glm::mat4 SpriteInvTranspModel = glm::mat4(glm::transpose(glm::inverse(spriteRenderer.m_RendererModelMatrix)));
	shader.setUniform("u_InvTranspModel", &SpriteInvTranspModel);
	shader.setUniform("u_Model", &spriteRenderer.m_RendererModelMatrix);
	spriteRenderer.drawPrimitives(shader);

	//Models - Make have per model inv transp later when models are used more
	modelAtlas.bind();
	modelRenderer.drawPrimitives(shader);

	//World
	worldTexture.bind();
	glm::mat4 WorldInvTranspModel = glm::mat4(glm::transpose(glm::inverse(worldRenderer.m_RendererModelMatrix)));
	shader.setUniform("u_InvTranspModel", &WorldInvTranspModel);
	shader.setUniform("u_Model", &worldRenderer.m_RendererModelMatrix);
	worldRenderer.drawPrimitives(shader);

	shader.unbind();
}