#include "game/states/StateRenderers.h"

void OverworldRenderer::initialiseRenderer(unsigned int width, unsigned int height, ObjectManager* obj)
{
	//Renderer setup
	camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

	//Object manager pointer
	objects = obj;

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
	//Renderers
	renderers.resize((int)StateRen::OVERWORLD_COUNT);

	//World Renderer
	this->at(StateRen::OVERWORLD).setLayout<float>(3, 2, 3);
	this->at(StateRen::OVERWORLD).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera);

	//Sprite Renderer
	this->at(StateRen::OVERWORLD_SPRITE).setLayout<float>(3, 2, 3);
	this->at(StateRen::OVERWORLD_SPRITE).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD_SPRITE).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera);

	//Pkm Renderer
	this->at(StateRen::OVERWORLD_POKEMON).setLayout<float>(3, 2, 3);
	this->at(StateRen::OVERWORLD_POKEMON).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD_POKEMON).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera);

	//Model Renderer
	this->at(StateRen::OVERWORLD_MODEL).setLayout<float>(3, 2, 3);
	this->at(StateRen::OVERWORLD_MODEL).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD_MODEL).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera);

	//Grass Renderer
	this->at(StateRen::OVERWORLD_GRASS).setLayout<float>(3, 2, 3);
	this->at(StateRen::OVERWORLD_GRASS).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD_GRASS).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera);

	//Shaders
	shaders.resize((int)StateShader::OVERWORLD_COUNT);
	shader(StateShader::OVERWORLD).create("res/shaders/Lighting_T_Shader.glsl");
	shader(StateShader::OVERWORLD_SHADOW).create("res/shaders/Shadows_Shader.glsl");
	shader(StateShader::OVERWORLD_GRASS).createGeo("res/shaders/Grass_Drawing_Shader.glsl");
	shader(StateShader::OVERWORLD_GRASS_SHADOW).createGeo("res/shaders/Grass_Shadows_Shader.glsl");
	shader(StateShader::OVERWORLD_BATTLE).create("res/shaders/Battle_Transition.glsl");
	shader(StateShader::OVERWORLD_FADE_OUT).create("res/shaders/Fade_Out_Transition.glsl");
	shader(StateShader::OVERWORLD_FADE_IN).create("res/shaders/Fade_In_Transition.glsl");

	//Textures
	textures.resize((int)StateTex::OVERWORLD_COUNT);

	//Load world texture
	texture(StateTex::OVERWORLD).loadTexture("res/textures/tilesets/0.png");
	texture(StateTex::OVERWORLD).generateTexture(OVERWORLD_TEXTURE_SLOT);
	texture(StateTex::OVERWORLD).bind();
	texture(StateTex::OVERWORLD).clearBuffer();

	//Load sprite textures
	texture(StateTex::OVERWORLD_SPRITE).loadTexture("res/textures/Sprite.png");
	texture(StateTex::OVERWORLD_SPRITE).generateTexture(OVERWORLD_TEXTURE_SLOT);
	texture(StateTex::OVERWORLD_SPRITE).bind();
	texture(StateTex::OVERWORLD_SPRITE).clearBuffer();

	//Load companion pkm texture
	texture(StateTex::OVERWORLD_POKEMON).loadTexture("res/textures/pokemon/overworld/bulbasaur.png");
	texture(StateTex::OVERWORLD_POKEMON).generateTexture(OVERWORLD_TEXTURE_SLOT);
	texture(StateTex::OVERWORLD_POKEMON).bind();
	texture(StateTex::OVERWORLD_POKEMON).clearBuffer();

	//Tilemaps
	tileMaps.resize((int)StateTileMap::OVERWORLD_COUNT);
	tilemap(StateTileMap::OVERWORLD) = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	tilemap(StateTileMap::OVERWORLD_SPRITE) = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	tilemap(StateTileMap::OVERWORLD_POKEMON) = TileMap::TileMap(texture(StateTex::OVERWORLD_POKEMON).width(), texture(StateTex::OVERWORLD_POKEMON).height(),
		(float)texture(StateTex::OVERWORLD_POKEMON).width() / 2.0f, (float)texture(StateTex::OVERWORLD_POKEMON).height() / 4.0f);

	//Load transitions
	transitions.resize((int)StateTrans::OVERWORLD_COUNT);
	transition(StateTrans::OVERWORLD_BATTLE).init((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
	transition(StateTrans::OVERWORLD_BATTLE).linkUniform("u_Height", &transition(StateTrans::OVERWORLD_BATTLE).m_Height);
	transition(StateTrans::OVERWORLD_BATTLE).setCap(1.0);

	transition(StateTrans::OVERWORLD_FADE_OUT).init((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
	transition(StateTrans::OVERWORLD_FADE_OUT).linkUniform("u_FadeCap", &fadeTime);
	transition(StateTrans::OVERWORLD_FADE_OUT).setCap(fadeTime);

	transition(StateTrans::OVERWORLD_FADE_IN).init((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
	transition(StateTrans::OVERWORLD_FADE_IN).linkUniform("u_FadeCap", &fadeTime);
	transition(StateTrans::OVERWORLD_FADE_IN).setCap(fadeTime);
}

void OverworldRenderer::generateAtlas()
{
	textureAtlas.generateAtlas();
	textureAtlas.generateTexture(OVERWORLD_TEXTURE_SLOT);
	textureAtlas.bind();
}

void OverworldRenderer::purgeData() 
{ 
	//Delete textures
	for (int i = 0; i < textures.size(); i++)
	{
		textures[i].deleteTexture();
	}
	textures.clear();

	//Clear and delete atlas
	textureAtlas.clearAtlasBuffers();
	textureAtlas.deleteTextures();

	//Delete shaders
	for (int i = 0; i < shaders.size(); i++)
	{
		shaders[i].deleteShader();
	}
	shaders.clear();

	//Delete renderers
	renderers.clear();

	//Delete transitions
	transitions.clear();

	//Delete tilemaps
	tileMaps.clear();

	readyToShow = false;
}

void OverworldRenderer::bufferRenderData()
{
	for (int i = 0; i < renderers.size(); i++)
	{
		renderers[i].bufferVideoData();
	}

	//If trigger to remap models sent, remap and buffer atlas
	if (m_RemapModels && m_LevelsLeftLoading <= 0)
	{
		mapModelTextures();
	}
}

void OverworldRenderer::mapModelTextures()
{
	//Request new textures
	textureAtlas.clearBuffers();
	textureAtlas.deleteTextures();
	std::vector<ObjectManager::GameObjectContainer>& objs = objects->getObjects();
	std::vector<ModelObject*> objPtrs;
	for (int i = 0; i < objs.size(); i++)
	{
		//If a model that isn't dead is found
		if (objs[i].obj->getTag() == (uint16_t)ObjectType::Model
			&& !objs[i].obj->isDead())
		{
			ModelObject* obj = (ModelObject*)objs[i].obj.get();
			textureAtlas.loadTexture("res/textures/" + obj->m_Texture, obj->m_Texture);
			objPtrs.push_back(obj);
		}
	}
	textureAtlas.generateAtlas();
	textureAtlas.generateTexture(OVERWORLD_TEXTURE_SLOT);
	textureAtlas.bind();
	for (int i = 0; i < objPtrs.size(); i++)
	{
		ModelObject* obj = objPtrs[i];
		Tex::TextureAtlasRGBA::unmapModelVerts(
			obj->m_Model.getVertices(),
			obj->m_Model.getVertCount(),
			obj->m_Texture,
			obj->m_LastTransform
		);
		obj->m_LastTransform = textureAtlas.mapModelVerts(
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
	for (int i = 0; i < transitions.size(); i++)
	{
		transitions[i].update(deltaTime);
	}
}

void OverworldRenderer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Handle fading
	if (transition(StateTrans::OVERWORLD_FADE_IN).isEnded())
	{

	}
	if (transition(StateTrans::OVERWORLD_FADE_OUT).isEnded())
	{
		readyToShow = false;
	}
	if (!readyToShow)
	{
		return;
	}

	//Send cam uniforms
	shader(StateShader::OVERWORLD).bind();
	camera.sendCameraUniforms(shader(StateShader::OVERWORLD));
	shader(StateShader::OVERWORLD_GRASS).bind();
	camera.sendCameraUniforms(shader(StateShader::OVERWORLD_GRASS));

	//Update player pos
	glm::vec3 lightDir = glm::normalize(m_LightDir);

	////Shadow pass
	shadowMap.bindForWriting();

	//Normal scene
	shader(StateShader::OVERWORLD_SHADOW).bind();

	//Use world renderer matrix - only works for sprite too as model matrixes are same (currently)
	glm::mat4 world = this->at(StateRen::OVERWORLD).m_RendererModelMatrix;
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightView = glm::translate(lightView, -camera.getPos());

	lightView[3][0] = floor(lightView[3][0]);
	lightView[3][1] = floor(lightView[3][1]);
	lightView[3][2] = floor(lightView[3][2]);

	shader(StateShader::OVERWORLD_SHADOW).setUniform("WVP", shadowMap.calcMVP(world, lightView));

	//Render from lights perspective
	shadowMap.startCapture();
	texture(StateTex::OVERWORLD).bind();
	this->at(StateRen::OVERWORLD).drawPrimitives();
	texture(StateTex::OVERWORLD_POKEMON).bind();
	this->at(StateRen::OVERWORLD_POKEMON).drawPrimitives();
	texture(StateTex::OVERWORLD_SPRITE).bind();
	this->at(StateRen::OVERWORLD_SPRITE).drawPrimitives();
	textureAtlas.bind();
	this->at(StateRen::OVERWORLD_MODEL).drawPrimitives();
	shader(StateShader::OVERWORLD_SHADOW).unbind();

	//Grass scene
	shader(StateShader::OVERWORLD_GRASS_SHADOW).bind();
	texture(StateTex::OVERWORLD).bind();
	shader(StateShader::OVERWORLD_GRASS_SHADOW).setUniform("WVP", shadowMap.calcMVP(world, lightView));
	this->at(StateRen::OVERWORLD_GRASS).drawPrimitives();
	shader(StateShader::OVERWORLD_GRASS_SHADOW).unbind();

	shadowMap.endCapture(SCREEN_WIDTH, SCREEN_HEIGHT);

	// reset viewport
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	////Lighting pass
	shader(StateShader::OVERWORLD).bind();
	shadowMap.bindForReading(OVERWORLD_SHADOWS_SLOT);

	//Scale lighting color
	m_LightScaled = m_LightColor * m_LightScaleFactor;

	//Set universal uniforms
	shader(StateShader::OVERWORLD).setUniform("u_AmbLight", &m_LightScaled);
	shader(StateShader::OVERWORLD).setUniform("u_LightDir", &lightDir);
	shader(StateShader::OVERWORLD).setUniform("u_LightMVP", shadowMap.getMVP());
	shader(StateShader::OVERWORLD).setUniform("u_ShadowMap", OVERWORLD_SHADOWS_SLOT);
	shader(StateShader::OVERWORLD).setUniform("u_LightsActive", lightScene);
	shader(StateShader::OVERWORLD).setUniform("u_Texture", OVERWORLD_TEXTURE_SLOT);

	//Sprites
	texture(StateTex::OVERWORLD_SPRITE).bind();
	glm::mat4 SpriteInvTranspModel = glm::mat4(glm::transpose(glm::inverse(this->at(StateRen::OVERWORLD_SPRITE).m_RendererModelMatrix)));
	shader(StateShader::OVERWORLD).setUniform("u_InvTranspModel", &SpriteInvTranspModel);
	shader(StateShader::OVERWORLD).setUniform("u_Model", &this->at(StateRen::OVERWORLD_SPRITE).m_RendererModelMatrix);
	this->at(StateRen::OVERWORLD_SPRITE).drawPrimitives();

	//Pokemon Sprite
	texture(StateTex::OVERWORLD_POKEMON).bind();
	glm::mat4 PkmInvTranspModel = glm::mat4(glm::transpose(glm::inverse(this->at(StateRen::OVERWORLD_POKEMON).m_RendererModelMatrix)));
	shader(StateShader::OVERWORLD).setUniform("u_InvTranspModel", &PkmInvTranspModel);
	shader(StateShader::OVERWORLD).setUniform("u_Model", &this->at(StateRen::OVERWORLD_POKEMON).m_RendererModelMatrix);
	this->at(StateRen::OVERWORLD_POKEMON).drawPrimitives();

	//Models - Make have per model inv transp later when models are used more
	textureAtlas.bind();
	this->at(StateRen::OVERWORLD_MODEL).drawPrimitives();

	//World
	texture(StateTex::OVERWORLD).bind();
	glm::mat4 WorldInvTranspModel = glm::mat4(glm::transpose(glm::inverse(this->at(StateRen::OVERWORLD).m_RendererModelMatrix)));
	shader(StateShader::OVERWORLD).setUniform("u_InvTranspModel", &WorldInvTranspModel);
	shader(StateShader::OVERWORLD).setUniform("u_Model", &this->at(StateRen::OVERWORLD).m_RendererModelMatrix);
	this->at(StateRen::OVERWORLD).drawPrimitives();
	shader(StateShader::OVERWORLD).unbind();

	//Grass
	shader(StateShader::OVERWORLD_GRASS).bind();
	shadowMap.bindForReading(OVERWORLD_SHADOWS_SLOT);

	shader(StateShader::OVERWORLD_GRASS).setUniform("u_AmbLight", &m_LightScaled);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_LightDir", &lightDir);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_LightMVP", shadowMap.getMVP());
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_ShadowMap", OVERWORLD_SHADOWS_SLOT);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_LightsActive", lightScene);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_Texture", OVERWORLD_TEXTURE_SLOT);

	shader(StateShader::OVERWORLD_GRASS).setUniform("u_InvTranspModel", &WorldInvTranspModel);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_Model", &this->at(StateRen::OVERWORLD).m_RendererModelMatrix);
	this->at(StateRen::OVERWORLD_GRASS).drawPrimitives();
	shader(StateShader::OVERWORLD_GRASS).unbind();

	//Transition
	transition(StateTrans::OVERWORLD_BATTLE).render(shader(StateShader::OVERWORLD_BATTLE));
	transition(StateTrans::OVERWORLD_FADE_OUT).render(shader(StateShader::OVERWORLD_FADE_OUT));
	transition(StateTrans::OVERWORLD_FADE_IN).render(shader(StateShader::OVERWORLD_FADE_IN));
}

//battle
void BattleRenderer::initialiseRenderer(unsigned int width, unsigned int height)
{
	//Renderer setup
	camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

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

	renderers.resize((int)StateRen::BATTLE_COUNT);

	//World Renderer
	this->at(StateRen::BATTLE_PLATFORM).setLayout<float>(3, 2);
	this->at(StateRen::BATTLE_PLATFORM).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::BATTLE_PLATFORM).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera);

	//Background Renderer
	this->at(StateRen::BATTLE_BACKGROUND).setLayout<float>(3, 2);
	this->at(StateRen::BATTLE_BACKGROUND).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::BATTLE_BACKGROUND).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera);

	//Pokemon renderer
	this->at(StateRen::BATTLE_POKEMONA).setLayout<float>(3, 2);
	this->at(StateRen::BATTLE_POKEMONA).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::BATTLE_POKEMONA).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera);
	this->at(StateRen::BATTLE_POKEMONB).setLayout<float>(3, 2);
	this->at(StateRen::BATTLE_POKEMONB).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::BATTLE_POKEMONB).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera);

	//Load world texture
	textures.resize((int)StateTex::BATTLE_COUNT);

	loadPokemonTextureA("bulbasaur");
	loadPokemonTextureB("bulbasaur");
	texture(StateTex::BATTLE_PLATFORM).loadTexture("res/textures/BattlePlatform.png");
	texture(StateTex::BATTLE_PLATFORM).generateTexture(BATTLE_TEXTURE_SLOT);
	texture(StateTex::BATTLE_PLATFORM).bind();
	texture(StateTex::BATTLE_PLATFORM).clearBuffer();

	//Load background
	texture(StateTex::BATTLE_BACKGROUND).loadTexture("res/textures/BattleBackground.png");
	texture(StateTex::BATTLE_BACKGROUND).generateTexture(BATTLE_TEXTURE_SLOT);
	texture(StateTex::BATTLE_BACKGROUND).bind();
	texture(StateTex::BATTLE_BACKGROUND).clearBuffer();
}

void BattleRenderer::purgeData()
{
	//Delete Renderers
	renderers.clear();

	//Delete Textures
	for (int i = 0; i < textures.size(); i++)
	{
		textures[i].deleteTexture();
	}
	textures.clear();
	
	//Delete shaders
	sceneShader.deleteShader();
}

void BattleRenderer::loadPokemonTextureA(std::string name)
{
	texture(StateTex::BATTLE_POKEMONA).loadTexture(BACK_TEX_PATH+name+PNG_EXT);
	texture(StateTex::BATTLE_POKEMONA).generateTexture(BATTLE_TEXTURE_SLOT);
	texture(StateTex::BATTLE_POKEMONA).bind();
	texture(StateTex::BATTLE_POKEMONA).clearBuffer();
}

void BattleRenderer::loadPokemonTextureB(std::string name)
{
	texture(StateTex::BATTLE_POKEMONB).loadTexture(FRONT_TEX_PATH+name+PNG_EXT);
	texture(StateTex::BATTLE_POKEMONB).generateTexture(BATTLE_TEXTURE_SLOT);
	texture(StateTex::BATTLE_POKEMONB).bind();
	texture(StateTex::BATTLE_POKEMONB).clearBuffer();
}

void BattleRenderer::bufferRenderData()
{
	for (int i = 0; i < renderers.size(); i++)
	{
		renderers[i].bufferVideoData();
	}
}

void BattleRenderer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Send cam uniforms
	sceneShader.bind();
	camera.sendCameraUniforms(sceneShader);

	//Set universal uniforms
	sceneShader.setUniform("u_Texture", BATTLE_TEXTURE_SLOT);
	for (int i = 0; i < renderers.size(); i++)
	{
		texture(i).bind();
		sceneShader.setUniform("u_Model", &renderers[i].m_RendererModelMatrix);
		renderers[i].drawPrimitives();
	}

	sceneShader.unbind();
}
