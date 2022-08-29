#include "game/states/StateRenderers.h"

void OverworldRenderer::initialiseRenderer(unsigned int width, unsigned int height, ObjectManager* obj)
{
	//Renderer setup
	camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

	//Object manager pointer
	objects = obj;

	//World Renderer
	worldRenderer.setLayout<float>(3, 2, 3);
	worldRenderer.setDrawingMode(GL_TRIANGLES);
	worldRenderer.generate((float)width, (float)height, &camera);

	//Sprite Renderer
	spriteRenderer.setLayout<float>(3, 2, 3);
	spriteRenderer.setDrawingMode(GL_TRIANGLES);
	spriteRenderer.generate((float)width, (float)height, &camera);

	//Pkm Renderer
	pokemonRenderer.setLayout<float>(3, 2, 3);
	pokemonRenderer.setDrawingMode(GL_TRIANGLES);
	pokemonRenderer.generate((float)width, (float)height, &camera);

	//Model Renderer
	modelRenderer.setLayout<float>(3, 2, 3);
	modelRenderer.setDrawingMode(GL_TRIANGLES);
	modelRenderer.generate((float)width, (float)height, &camera);

	//Grass Renderer
	grassRenderer.setLayout<float>(3, 2, 3);
	grassRenderer.setDrawingMode(GL_TRIANGLES);
	grassRenderer.generate((float)width, (float)height, &camera);
	
	//Camera
	camera.setProjection(glm::perspective(glm::radians(45.0f), (float)((float)width / (float)height), 0.1f, 1000.0f));
	camera.moveUp(World::TILE_SIZE * 5);
	camera.panYDegrees(45.0f);

	//Shadow Map - Will improve moving object edge flickering TODO
	shadowMap.init();
	float sWidth = pow(2, floor(log(width) / log(2)));
	float sHeight = pow(2, floor(log(height) / log(2)));
	glm::mat4 lightProj = glm::ortho(-sWidth, sWidth, -sHeight, sHeight, -100.0f, 750.0f);
	shadowMap.setProjection(lightProj);

	SCREEN_HEIGHT = height; SCREEN_WIDTH = width;
}

void OverworldRenderer::loadRendererData()
{
	//Shader
	shaders.resize(OVERWORLD_SHADER_COUNT);
	shaders[OVERWORLD_SHADER].create("res/shaders/Lighting_T_Shader.glsl");
	shaders[OVERWORLD_SHADOW_SHADER].create("res/shaders/Shadows_Shader.glsl");
	shaders[GRASS_SHADER].createGeo("res/shaders/Grass_Drawing_Shader.glsl");
	shaders[GRASS_SHADOW_SHADER].createGeo("res/shaders/Grass_Shadows_Shader.glsl");
	shaders[BATTLE_TRANSITION_SHADER].create("res/shaders/Battle_Transition.glsl");
	shaders[FADE_OUT_SHADER].create("res/shaders/Fade_Out_Transition.glsl");
	shaders[FADE_IN_SHADER].create("res/shaders/Fade_In_Transition.glsl");

	//Load world texture
	worldTexture.loadTexture("res/textures/tilesets/0.png");
	worldTexture.generateTexture(OVERWORLD_TEXTURE_SLOT);
	worldTexture.bind();
	worldTexture.clearBuffer();

	//Load sprite textures
	spriteTexture.loadTexture("res/textures/Sprite.png");
	spriteTexture.generateTexture(OVERWORLD_TEXTURE_SLOT);
	spriteTexture.bind();
	spriteTexture.clearBuffer();

	//Load companion pkm texture
	pokemonTexture.loadTexture("res/textures/pokemon/overworld/bulbasaur.png");
	pokemonTexture.generateTexture(OVERWORLD_TEXTURE_SLOT);
	pokemonTexture.bind();
	pokemonTexture.clearBuffer();
	pokemonTileMap = TileMap::TileMap(pokemonTexture.width(), pokemonTexture.height(), (float)pokemonTexture.width() / 2.0f, (float)pokemonTexture.height() / 4.0f);

	//Load transitions
	battleTransition = Transition();
	battleTransition.init((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
	battleTransition.linkUniform("u_Height", &battleTransition.m_Height);
	battleTransition.setCap(1.0);

	fadeOut = Transition();
	fadeOut.init((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
	fadeOut.linkUniform("u_FadeCap", &m_FadeTime);
	fadeOut.setCap(m_FadeTime);

	fadeIn = Transition();
	fadeIn.init((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
	fadeIn.linkUniform("u_FadeCap", &m_FadeTime);
	fadeIn.setCap(m_FadeTime);
}

void OverworldRenderer::generateAtlas()
{
	modelAtlas.generateAtlas();
	modelAtlas.generateTexture(OVERWORLD_TEXTURE_SLOT);
	modelAtlas.bind();
}

void OverworldRenderer::purgeData() 
{ 
	//Delete textures
	worldTexture.deleteTexture();
	spriteTexture.deleteTexture();
	pokemonTexture.deleteTexture();
	modelAtlas.clearAtlasBuffers();
	modelAtlas.deleteTextures();

	//Delete shaders
	for (int i = 0; i < shaders.size(); i++)
	{
		shaders[i].deleteShader();
	}
	shaders.clear();
}

void OverworldRenderer::bufferRenderData()
{
	spriteRenderer.bufferVideoData();
	pokemonRenderer.bufferVideoData();
	modelRenderer.bufferVideoData();
	worldRenderer.bufferVideoData();
	grassRenderer.bufferVideoData();

	//If trigger to remap models sent, remap and buffer atlas
	if (m_RemapModels && m_LevelsLeftLoading <= 0)
	{
		mapModelTextures();
	}
}

void OverworldRenderer::mapModelTextures()
{
	//Request new textures
	modelAtlas.clearBuffers();
	modelAtlas.deleteTextures();
	std::vector<ObjectManager::GameObjectContainer>& objs = objects->getObjects();
	std::vector<ModelObject*> objPtrs;
	for (int i = 0; i < objs.size(); i++)
	{
		//If a model that isn't dead is found
		if (objs[i].obj->getTag() == (uint16_t)ObjectType::Model
			&& !objs[i].obj->isDead())
		{
			ModelObject* obj = (ModelObject*)objs[i].obj.get();
			modelAtlas.loadTexture("res/textures/" + obj->m_Texture, obj->m_Texture);
			objPtrs.push_back(obj);
		}
	}
	modelAtlas.generateAtlas();
	modelAtlas.generateTexture(OVERWORLD_TEXTURE_SLOT);
	modelAtlas.bind();
	for (int i = 0; i < objPtrs.size(); i++)
	{
		ModelObject* obj = objPtrs[i];
		Tex::TextureAtlasRGBA::unmapModelVerts(
			obj->m_Model.getVertices(),
			obj->m_Model.getVertCount(),
			obj->m_Texture,
			obj->m_LastTransform
		);
		obj->m_LastTransform = modelAtlas.mapModelVerts(
			obj->m_Model.getVertices(),
			obj->m_Model.getVertCount(),
			obj->m_Texture
		);
	}
	m_RemapModels = false;
	m_LevelsLeftLoading = 0;
}

void OverworldRenderer::update(double deltaTime)
{
	battleTransition.update(deltaTime);
	fadeOut.update(deltaTime);
	fadeIn.update(deltaTime);
}

void OverworldRenderer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Handle fading
	if (fadeIn.isEnded())
	{

	}
	if (fadeOut.isEnded())
	{
		m_ReadyToShow = false;
	}
	if (!m_ReadyToShow)
	{
		return;
	}

	//Send cam uniforms
	shaders[OVERWORLD_SHADER].bind();
	camera.sendCameraUniforms(shaders[OVERWORLD_SHADER]);
	shaders[GRASS_SHADER].bind();
	camera.sendCameraUniforms(shaders[GRASS_SHADER]);

	//Update player pos
	glm::vec3 lightDir = glm::normalize(m_LightDir);

	////Shadow pass
	shadowMap.bindForWriting();

	//Normal scene
	shaders[OVERWORLD_SHADOW_SHADER].bind();

	//Use world renderer matrix - only works for sprite too as model matrixes are same (currently)
	glm::mat4 world = worldRenderer.m_RendererModelMatrix;
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightView = glm::translate(lightView, -camera.getPos());

	lightView[3][0] = floor(lightView[3][0]);
	lightView[3][1] = floor(lightView[3][1]);
	lightView[3][2] = floor(lightView[3][2]);

	shaders[OVERWORLD_SHADOW_SHADER].setUniform("WVP", shadowMap.calcMVP(world, lightView));

	//Render from lights perspective
	shadowMap.startCapture();
	worldTexture.bind();
	worldRenderer.drawPrimitives();
	pokemonTexture.bind();
	pokemonRenderer.drawPrimitives();
	spriteTexture.bind();
	spriteRenderer.drawPrimitives();
	modelAtlas.bind();
	modelRenderer.drawPrimitives();
	shaders[OVERWORLD_SHADOW_SHADER].unbind();

	//Grass scene
	shaders[GRASS_SHADOW_SHADER].bind();
	worldTexture.bind();
	shaders[GRASS_SHADOW_SHADER].setUniform("WVP", shadowMap.calcMVP(world, lightView));
	grassRenderer.drawPrimitives();
	shaders[GRASS_SHADOW_SHADER].unbind();

	shadowMap.endCapture(SCREEN_WIDTH, SCREEN_HEIGHT);

	// reset viewport
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	////Lighting pass
	shaders[OVERWORLD_SHADER].bind();
	shadowMap.bindForReading(OVERWORLD_SHADOWS_SLOT);

	//Scale lighting color
	m_LightScaled = m_LightColor * m_LightScaleFactor;

	//Set universal uniforms
	shaders[OVERWORLD_SHADER].setUniform("u_AmbLight", &m_LightScaled);
	shaders[OVERWORLD_SHADER].setUniform("u_LightDir", &lightDir);
	shaders[OVERWORLD_SHADER].setUniform("u_LightMVP", shadowMap.getMVP());
	shaders[OVERWORLD_SHADER].setUniform("u_ShadowMap", OVERWORLD_SHADOWS_SLOT);
	shaders[OVERWORLD_SHADER].setUniform("u_LightsActive", lightScene);
	shaders[OVERWORLD_SHADER].setUniform("u_Texture", OVERWORLD_TEXTURE_SLOT);

	//Sprites
	spriteTexture.bind();
	glm::mat4 SpriteInvTranspModel = glm::mat4(glm::transpose(glm::inverse(spriteRenderer.m_RendererModelMatrix)));
	shaders[OVERWORLD_SHADER].setUniform("u_InvTranspModel", &SpriteInvTranspModel);
	shaders[OVERWORLD_SHADER].setUniform("u_Model", &spriteRenderer.m_RendererModelMatrix);
	spriteRenderer.drawPrimitives();

	//Pokemon Sprite
	pokemonTexture.bind();
	glm::mat4 PkmInvTranspModel = glm::mat4(glm::transpose(glm::inverse(pokemonRenderer.m_RendererModelMatrix)));
	shaders[OVERWORLD_SHADER].setUniform("u_InvTranspModel", &PkmInvTranspModel);
	shaders[OVERWORLD_SHADER].setUniform("u_Model", &spriteRenderer.m_RendererModelMatrix);
	pokemonRenderer.drawPrimitives();

	//Models - Make have per model inv transp later when models are used more
	modelAtlas.bind();
	modelRenderer.drawPrimitives();

	//World
	worldTexture.bind();
	glm::mat4 WorldInvTranspModel = glm::mat4(glm::transpose(glm::inverse(worldRenderer.m_RendererModelMatrix)));
	shaders[OVERWORLD_SHADER].setUniform("u_InvTranspModel", &WorldInvTranspModel);
	shaders[OVERWORLD_SHADER].setUniform("u_Model", &worldRenderer.m_RendererModelMatrix);
	worldRenderer.drawPrimitives();
	shaders[OVERWORLD_SHADER].unbind();

	//Grass
	shaders[GRASS_SHADER].bind();
	shadowMap.bindForReading(OVERWORLD_SHADOWS_SLOT);

	shaders[GRASS_SHADER].setUniform("u_AmbLight", &m_LightScaled);
	shaders[GRASS_SHADER].setUniform("u_LightDir", &lightDir);
	shaders[GRASS_SHADER].setUniform("u_LightMVP", shadowMap.getMVP());
	shaders[GRASS_SHADER].setUniform("u_ShadowMap", OVERWORLD_SHADOWS_SLOT);
	shaders[GRASS_SHADER].setUniform("u_LightsActive", lightScene);
	shaders[GRASS_SHADER].setUniform("u_Texture", OVERWORLD_TEXTURE_SLOT);

	shaders[GRASS_SHADER].setUniform("u_InvTranspModel", &WorldInvTranspModel);
	shaders[GRASS_SHADER].setUniform("u_Model", &worldRenderer.m_RendererModelMatrix);
	grassRenderer.drawPrimitives();
	shaders[GRASS_SHADER].unbind();

	//Transition
	battleTransition.render(shaders[BATTLE_TRANSITION_SHADER]);
	fadeOut.render(shaders[FADE_OUT_SHADER]);
	fadeIn.render(shaders[FADE_IN_SHADER]);
}

//battle
void BattleRenderer::initialiseRenderer(unsigned int width, unsigned int height)
{
	//Renderer setup
	camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

	//World Renderer
	worldRenderer.setLayout<float>(3, 2);
	worldRenderer.setDrawingMode(GL_TRIANGLES);
	worldRenderer.generate((float)width, (float)height, &camera);

	//Background Renderer
	backgroundRenderer.setLayout<float>(3, 2);
	backgroundRenderer.setDrawingMode(GL_TRIANGLES);
	backgroundRenderer.generate((float)width, (float)height, &camera);

	//Pokemon renderer
	pokemonARenderer.setLayout<float>(3, 2);
	pokemonARenderer.setDrawingMode(GL_TRIANGLES);
	pokemonARenderer.generate((float)width, (float)height, &camera);
	pokemonBRenderer.setLayout<float>(3, 2);
	pokemonBRenderer.setDrawingMode(GL_TRIANGLES);
	pokemonBRenderer.generate((float)width, (float)height, &camera);

	//Camera
	camera.setProjection(glm::perspective(glm::radians(45.0f), (float)((float)width / (float)height), 0.1f, 4000.0f));
	camera.moveUp((float)width/19.2f);
	camera.moveZ((float)width / -22.2f);
	camera.moveX((float)width / -38.4f);
	camera.panYDegrees(15.0f);

	SCREEN_HEIGHT = height; SCREEN_WIDTH = width;
}

void BattleRenderer::loadRendererData()
{
	//Shader
	sceneShader.create("res/shaders/Default_T_Shader.glsl");

	//Load world texture
	loadPokemonTextureA("bulbasaur");
	loadPokemonTextureB("bulbasaur");
	platformTexture.loadTexture("res/textures/BattlePlatform.png");
	platformTexture.generateTexture(BATTLE_TEXTURE_SLOT);
	platformTexture.bind();
	platformTexture.clearBuffer();

	//Load background
	backgroundTexture.loadTexture("res/textures/BattleBackground.png");
	backgroundTexture.generateTexture(BATTLE_TEXTURE_SLOT);
	backgroundTexture.bind();
	backgroundTexture.clearBuffer();
}

void BattleRenderer::purgeData()
{
	//Delete Textures
	platformTexture.deleteTexture();
	backgroundTexture.deleteTexture();
	pokemonATexture.deleteTexture();
	pokemonBTexture.deleteTexture();
	
	//Delete shaders
	sceneShader.deleteShader();
}

void BattleRenderer::loadPokemonTextureA(std::string name)
{
	pokemonATexture.loadTexture(BACK_TEX_PATH+name+PNG_EXT);
	pokemonATexture.generateTexture(BATTLE_TEXTURE_SLOT);
	pokemonATexture.bind();
	pokemonATexture.clearBuffer();
}

void BattleRenderer::loadPokemonTextureB(std::string name)
{
	pokemonBTexture.loadTexture(FRONT_TEX_PATH+name+PNG_EXT);
	pokemonBTexture.generateTexture(BATTLE_TEXTURE_SLOT);
	pokemonBTexture.bind();
	pokemonBTexture.clearBuffer();
}

void BattleRenderer::bufferRenderData()
{
	worldRenderer.bufferVideoData();
	backgroundRenderer.bufferVideoData();
	pokemonARenderer.bufferVideoData();
	pokemonBRenderer.bufferVideoData();
}

void BattleRenderer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Send cam uniforms
	sceneShader.bind();
	camera.sendCameraUniforms(sceneShader);

	//Set universal uniforms
	sceneShader.setUniform("u_Texture", BATTLE_TEXTURE_SLOT);

	//World
	platformTexture.bind();
	sceneShader.setUniform("u_Model", &worldRenderer.m_RendererModelMatrix);
	worldRenderer.drawPrimitives();
	sceneShader.unbind();

	//Background
	backgroundTexture.bind();
	sceneShader.setUniform("u_Model", &backgroundRenderer.m_RendererModelMatrix);
	backgroundRenderer.drawPrimitives();
	sceneShader.unbind();

	//Sprite A
	pokemonATexture.bind();
	sceneShader.setUniform("u_Model", &pokemonARenderer.m_RendererModelMatrix);
	pokemonARenderer.drawPrimitives();

	//Sprite B
	pokemonBTexture.bind();
	sceneShader.setUniform("u_Model", &pokemonBRenderer.m_RendererModelMatrix);
	pokemonBRenderer.drawPrimitives();

	sceneShader.unbind();
}
