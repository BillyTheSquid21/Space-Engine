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

	//Grass Renderer
	grassRenderer.setLayout<float>(3, 2, 3);
	grassRenderer.setDrawingMode(GL_TRIANGLES);
	grassRenderer.generate((float)width, (float)height, &camera);

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
	sceneShader.create("res/shaders/Lighting_T_Shader.glsl");
	sceneShadows.create("res/shaders/Shadows_Shader.glsl");
	grassShader.createGeo("res/shaders/Grass_Drawing_Shader.glsl");
	grassShadows.createGeo("res/shaders/Grass_Shadows_Shader.glsl");

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

	//Model debug texture
	debugTexture.loadTexture("res/textures/Debug.png");
	debugTexture.generateTexture(TEXTURE_SLOT);
	debugTexture.bind();
	debugTexture.clearBuffer();
	model.setRen(&modelRenderer);
	Translate<NormalTextureVertex>(model.getVertices(), 278, 0, -64, model.getVertCount());
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
	grassRenderer.bufferVideoData();
}

void OverworldRenderer::draw()
{
	model.render();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Send cam uniforms
	sceneShader.bind();
	camera.sendCameraUniforms(sceneShader);
	grassShader.bind();
	camera.sendCameraUniforms(grassShader);

	//Update player pos
	glm::vec3 lightDir = glm::normalize(m_LightDir);

	////Shadow pass
	shadowMap.bindForWriting();

	//Normal scene
	sceneShadows.bind();

	//Use world renderer matrix - only works for sprite too as model matrixes are same (currently)
	glm::mat4 world = worldRenderer.m_RendererModelMatrix;
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightView = glm::translate(lightView, -camera.getPos());
	sceneShadows.setUniform("WVP", shadowMap.calcMVP(world, lightView));
	
	//Render from lights perspective
	shadowMap.startCapture();
	worldTexture.bind();
	worldRenderer.drawPrimitives(sceneShadows);
	spriteTexture.bind();
	spriteRenderer.drawPrimitives(sceneShadows);
	debugTexture.bind();
	modelRenderer.drawPrimitives(sceneShadows);
	sceneShadows.unbind();

	//Grass scene
	grassShadows.bind();
	worldTexture.bind();
	grassShadows.setUniform("WVP", shadowMap.calcMVP(world, lightView));
	grassRenderer.drawPrimitives(grassShadows);
	grassShadows.unbind();

	shadowMap.endCapture(SCREEN_WIDTH, SCREEN_HEIGHT);

	// reset viewport
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	////Lighting pass
	sceneShader.bind();
	shadowMap.bindForReading(SHADOWS_SLOT);

	//Scale lighting color
	m_LightScaled = m_LightColor * m_LightScaleFactor;

	//Set universal uniforms
	sceneShader.setUniform("u_AmbLight", &m_LightScaled);
	sceneShader.setUniform("u_LightDir", &lightDir);
	sceneShader.setUniform("u_LightMVP", shadowMap.getMVP());
	sceneShader.setUniform("u_ShadowMap", SHADOWS_SLOT);
	sceneShader.setUniform("u_LightsActive", lightScene);
	sceneShader.setUniform("u_Texture", TEXTURE_SLOT);

	//Sprites
	spriteTexture.bind();
	glm::mat4 SpriteInvTranspModel = glm::mat4(glm::transpose(glm::inverse(spriteRenderer.m_RendererModelMatrix)));
	sceneShader.setUniform("u_InvTranspModel", &SpriteInvTranspModel);
	sceneShader.setUniform("u_Model", &spriteRenderer.m_RendererModelMatrix);
	spriteRenderer.drawPrimitives(sceneShader);

	//Models - Make have per model inv transp later when models are used more
	modelAtlas.bind();
	debugTexture.bind();
	modelRenderer.drawPrimitives(sceneShader);

	//World
	worldTexture.bind();
	glm::mat4 WorldInvTranspModel = glm::mat4(glm::transpose(glm::inverse(worldRenderer.m_RendererModelMatrix)));
	sceneShader.setUniform("u_InvTranspModel", &WorldInvTranspModel);
	sceneShader.setUniform("u_Model", &worldRenderer.m_RendererModelMatrix);
	worldRenderer.drawPrimitives(sceneShader);
	sceneShader.unbind();

	//Grass
	grassShader.bind();
	shadowMap.bindForReading(SHADOWS_SLOT);

	grassShader.setUniform("u_AmbLight", &m_LightScaled);
	grassShader.setUniform("u_LightDir", &lightDir);
	grassShader.setUniform("u_LightMVP", shadowMap.getMVP());
	grassShader.setUniform("u_ShadowMap", SHADOWS_SLOT);
	grassShader.setUniform("u_LightsActive", lightScene);
	grassShader.setUniform("u_Texture", TEXTURE_SLOT);

	grassShader.setUniform("u_InvTranspModel", &WorldInvTranspModel);
	grassShader.setUniform("u_Model", &worldRenderer.m_RendererModelMatrix);
	grassRenderer.drawPrimitives(grassShader);
	grassShader.unbind();
}