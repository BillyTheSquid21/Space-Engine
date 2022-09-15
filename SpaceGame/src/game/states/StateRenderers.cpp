#include "game/states/StateRenderers.h"

void StateRender::Overworld::initialiseRenderer(unsigned int width, unsigned int height, SGObject::ObjectManager* obj)
{
	//Renderer setup
	camera = SGRender::Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

	//Object manager and player ptrs
	m_Objects = obj;

	//Camera
	camera.setProjection(glm::perspective(glm::radians(45.0f), (float)((float)width / (float)height), 0.1f, 1000.0f));
	camera.setFOV(45.0f);
	camera.moveUp(World::TILE_SIZE * 5);
	camera.panYDegrees(45.0f);

	//Shadow Map
	float sWidth = 2048;
	float sHeight = 2048;
	m_ShadowMap = SGRender::ShadowMap(2*sWidth, 2*sHeight);
	m_ShadowMap.init();
	glm::mat4 lightProj = glm::ortho(-sWidth, sWidth, -sHeight, sHeight, -100.0f, 750.0f);
	m_ShadowMap.setProjection(lightProj);

	m_Pool = MtLib::ThreadPool::Fetch();

	//Init grass
	m_Grass[0].position -= glm::vec3(0.0f, 0.0f, 3.0f);
	m_Grass[1].position -= glm::vec3(0.0f, 0.0f, 3.0f);
	m_Grass[2].color *= glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	m_Grass[3].color *= glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	SCREEN_HEIGHT = height; SCREEN_WIDTH = width;
}

void StateRender::Overworld::loadRendererData()
{
	using namespace SGRender;
	//Renderers
	m_Renderers.resize((int)StateRen::OVERWORLD_COUNT);

	//World Renderer
	this->at(StateRen::OVERWORLD).setLayout<float>(3, 2, 3);
	this->at(StateRen::OVERWORLD).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera, sizeof(NTVertex));

	//Sprite Renderer
	this->at(StateRen::OVERWORLD_SPRITE).setLayout<float>(3, 2, 3);
	this->at(StateRen::OVERWORLD_SPRITE).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD_SPRITE).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera, sizeof(NTVertex));

	//Pkm Renderer
	this->at(StateRen::OVERWORLD_POKEMON).setLayout<float>(3, 2, 3);
	this->at(StateRen::OVERWORLD_POKEMON).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD_POKEMON).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera, sizeof(NTVertex));

	//Model Renderer
	this->at(StateRen::OVERWORLD_MODEL).setLayout<float>(3, 2, 3);
	this->at(StateRen::OVERWORLD_MODEL).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD_MODEL).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera, sizeof(NTVertex));

	//Grass Renderer
	this->at(StateRen::OVERWORLD_GRASS).setLayout<float>(3, 4);
	this->at(StateRen::OVERWORLD_GRASS).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD_GRASS).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera, sizeof(NTVertex));
	this->at(StateRen::OVERWORLD_GRASS).addInstances<float>(sizeof(glm::vec4), 4, 2);

	//Tree Renderer
	this->at(StateRen::OVERWORLD_TREE).setLayout<float>(3, 2, 3);
	this->at(StateRen::OVERWORLD_TREE).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::OVERWORLD_TREE).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera, sizeof(NTVertex));

	//Shaders
	m_Shaders.resize((int)StateShader::OVERWORLD_COUNT);
	
	shader(StateShader::OVERWORLD).create(
		"res/shaders/vert/Lighting_T_Shader.vert",
		"res/shaders/frag/Lighting_T_Shader.frag"
	);
	
	shader(StateShader::OVERWORLD_SHADOW).create(
		"res/shaders/vert/Shadow_Shader.vert",
		"res/shaders/frag/Shadow_Shader.frag"
	);
	
	shader(StateShader::OVERWORLD_GRASS).create(
		"res/shaders/vert/Grass_Draw_Shader.vert",
		"res/shaders/geo/Grass_Draw_Shader.geom",
		"res/shaders/frag/Lighting_C_Shader.frag"
	);

	shader(StateShader::OVERWORLD_GRASS_SHADOW).create(
		"res/shaders/vert/Grass_Shadow_Shader.vert",
		"res/shaders/geo/Grass_Shadow_Shader.geom",
		"res/shaders/frag/Shadow_Shader.frag"
	);
	
	shader(StateShader::OVERWORLD_BATTLE).create(
		"res/shaders/vert/Transition.vert",
		"res/shaders/frag/Battle_Transition.frag"
	);

	shader(StateShader::OVERWORLD_FADE_OUT).create(
		"res/shaders/vert/Transition.vert",
		"res/shaders/frag/Fade_Out_Transition.frag"
	);

	shader(StateShader::OVERWORLD_FADE_IN).create(
		"res/shaders/vert/Transition.vert",
		"res/shaders/frag/Fade_In_Transition.frag"
	);

	shader(StateShader::OVERWORLD_TREE).create(
		"res/shaders/vert/Lighting_T_Geom_Shader.vert",
		"res/shaders/geo/Tree_Draw_Shader.geom",
		"res/shaders/frag/Lighting_T_Shader.frag"
	);

	shader(StateShader::OVERWORLD_TREE_SHADOW).create(
		"res/shaders/vert/Geom_Shadow_Shader.vert",
		"res/shaders/geo/Tree_Shadow_Shader.geom",
		"res/shaders/frag/Shadow_Shader.frag"
	);

	//Textures
	m_Textures.resize((int)StateTex::OVERWORLD_COUNT);

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

	//Init wind
	m_PerlinGenerator = SGRandom::Perlin2D<uint8_t, 3, 17, 512>::Perlin2D(false, (uint8_t)0, (uint8_t)255, (uint8_t)120, (uint8_t)135, (uint8_t)0, (uint8_t)255);
	m_PerlinGenerator.randomize(10000000.0f);
	m_BufferA.resize(m_PerlinGenerator.dataSizeBytes());
	m_BufferB.resize(m_PerlinGenerator.dataSizeBytes());
	m_ScrollNoise = std::bind(&SGRandom::Perlin2D<uint8_t, 3, 17, 512>::scrollNoise, &m_PerlinGenerator, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	
	//Write into buffer A first
	memcpy(&m_BufferA[0], m_PerlinGenerator.data(), m_PerlinGenerator.dataSizeBytes());

	//Scroll and write into buffer B
	m_PerlinGenerator.scrollNoise(1, 0, 1000000.0f);
	memcpy(&m_BufferB[0], m_PerlinGenerator.data(), m_PerlinGenerator.dataSizeBytes());

	//Start scrolling for next buffer
	m_Pool->RunAndReturn(m_ScrollNoise, &m_NoiseReady, 1, 0, 1000000.0f);

	//Init wind textures
	texture(StateTex::OVERWORLD_WIND_A).setWidth(512); texture(StateTex::OVERWORLD_WIND_A).setHeight(512);
	texture(StateTex::OVERWORLD_WIND_A).setBPP(3);
	texture(StateTex::OVERWORLD_WIND_A).generateTextureWrap(OVERWORLD_WIND_SLOT_A, &m_BufferA[0]);

	texture(StateTex::OVERWORLD_WIND_B).setWidth(512); texture(StateTex::OVERWORLD_WIND_B).setHeight(512);
	texture(StateTex::OVERWORLD_WIND_B).setBPP(3);
	texture(StateTex::OVERWORLD_WIND_B).generateTextureWrap(OVERWORLD_WIND_SLOT_B, &m_BufferB[0]);

	//Tilemaps
	m_TileMaps.resize((int)StateTileMap::OVERWORLD_COUNT);
	tilemap(StateTileMap::OVERWORLD) = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	tilemap(StateTileMap::OVERWORLD_SPRITE) = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	tilemap(StateTileMap::OVERWORLD_POKEMON) = TileMap::TileMap(texture(StateTex::OVERWORLD_POKEMON).width(), texture(StateTex::OVERWORLD_POKEMON).height(),
		(float)texture(StateTex::OVERWORLD_POKEMON).width() / 2.0f, (float)texture(StateTex::OVERWORLD_POKEMON).height() / 4.0f);

	//Load transitions
	m_Transitions.resize((int)StateTrans::OVERWORLD_COUNT);
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

void StateRender::Overworld::generateAtlas()
{
	m_TextureAtlas.generateAtlas();
	m_TextureAtlas.generateTexture(OVERWORLD_TEXTURE_SLOT);
	m_TextureAtlas.bind();
}

void StateRender::Overworld::purgeData()
{ 
	//Delete textures
	for (int i = 0; i < m_Textures.size(); i++)
	{
		texture(i).deleteTexture();
	}
	m_Textures.clear();

	//Clear and delete atlas
	m_TextureAtlas.clearAtlasBuffers();
	m_TextureAtlas.deleteTextures();

	//Delete shaders
	for (int i = 0; i < m_Shaders.size(); i++)
	{
		shader(i).deleteShader();
	}
	m_Shaders.clear();
	m_Renderers.clear();
	m_Transitions.clear();
	m_TileMaps.clear();
	m_PerlinGenerator.clear();

	readyToShow = false;
}

void StateRender::Overworld::bufferRenderData()
{
	//Commit grass blade quad
	this->at(StateRen::OVERWORLD_GRASS).commit(&m_Grass[0], Geometry::GetFloatCount<SGRender::CVertex>(Geometry::Shape::QUAD), &Primitive::Q_IND[0], Primitive::Q_IND_COUNT);
	for (int i = 0; i < m_Renderers.size(); i++)
	{
		this->at(i).bufferVideoData();
	}

	//If trigger to remap models sent, remap and buffer atlas
	if (m_RemapModels && m_LevelsLeftLoading <= 0)
	{
		mapModelTextures();
	}
}

void StateRender::Overworld::mapModelTextures()
{
	using namespace SGObject;
	//Request new textures
	m_TextureAtlas.clearBuffers();
	m_TextureAtlas.deleteTextures();
	std::vector<ObjectManager::GameObjectContainer>& objs = m_Objects->getObjects();
	std::vector<ModelObject*> objPtrs;
	for (int i = 0; i < objs.size(); i++)
	{
		//If a model that isn't dead is found
		if (objs[i].obj->getTag() == (uint16_t)ObjectType::Model
			&& !objs[i].obj->isDead())
		{
			ModelObject* obj = (ModelObject*)objs[i].obj.get();
			m_TextureAtlas.loadTexture("res/textures/" + obj->m_Texture, obj->m_Texture);
			objPtrs.push_back(obj);
		}
	}
	m_TextureAtlas.generateAtlas();
	m_TextureAtlas.generateTexture(OVERWORLD_TEXTURE_SLOT);
	m_TextureAtlas.bind();
	for (int i = 0; i < objPtrs.size(); i++)
	{
		ModelObject* obj = objPtrs[i];
		Tex::TextureAtlasRGBA::unmapModelVerts(
			obj->m_Model.getVertices(),
			obj->m_Model.getVertCount(),
			obj->m_Texture,
			obj->m_LastTransform
		);
		obj->m_LastTransform = m_TextureAtlas.mapModelVerts(
			obj->m_Model.getVertices(),
			obj->m_Model.getVertCount(),
			obj->m_Texture
		);
	}
	m_RemapModels = false;
	m_LevelsLeftLoading = 0;
}

void StateRender::Overworld::handleWind()
{
	//Change weighting of wind buffers depending on current buffer
	if (m_CurrBuffer == 0)
	{
		m_WindWeightA = 1.0f - (m_WindTimer / m_WindSampleInterval);
	}
	else if (m_CurrBuffer == 1)
	{
		m_WindWeightA = m_WindTimer / m_WindSampleInterval;
	}
	if (m_WindWeightA < 0.0f)
	{
		m_WindWeightA = 0.0f;
	}
	else if (m_WindWeightA > 1.0f)
	{
		m_WindWeightA = 1.0f;
	}

	if (m_WindTimer > m_WindSampleInterval && m_NoiseReady)
	{
		//Check which buffer to write to
		//Write to buffer with oldest data
		if (m_CurrBuffer == 0)
		{
			memcpy(&m_BufferA[0], m_PerlinGenerator.data(), m_PerlinGenerator.dataSizeBytes());
			texture(StateTex::OVERWORLD_WIND_A).deleteTexture();
			texture(StateTex::OVERWORLD_WIND_A).generateTextureWrap(OVERWORLD_WIND_SLOT_A, &m_BufferA[0]);
			texture(StateTex::OVERWORLD_WIND_A).bind();
			m_WindWeightA = 0.0f;
			m_CurrBuffer = 1;
		}
		else if (m_CurrBuffer == 1)
		{
			memcpy(&m_BufferB[0], m_PerlinGenerator.data(), m_PerlinGenerator.dataSizeBytes());
			texture(StateTex::OVERWORLD_WIND_B).deleteTexture();
			texture(StateTex::OVERWORLD_WIND_B).generateTextureWrap(OVERWORLD_WIND_SLOT_B, &m_BufferB[0]);
			texture(StateTex::OVERWORLD_WIND_B).bind();
			m_WindWeightA = 1.0f;
			m_CurrBuffer = 0;
		}

		//Start scrolling texture next
		m_Pool->RunAndReturn(m_ScrollNoise, &m_NoiseReady, 1, 0, 1000000.0f);
		m_WindTimer = 0;
	}
}

void StateRender::Overworld::updatePlayerPos()
{
	//Update player position - TODO use glm::vec3 for player pos in sprite
	if (m_PlayerPtr)
	{
		//Update pure pointer to be slightly behind sprite for grass
		m_PlayerPos = { m_PlayerPtr->m_XPos, m_PlayerPtr->m_YPos, m_PlayerPtr->m_ZPos - 10.0f };

		//For horizontal sprinting skew displacement ahead of player
		if (m_PlayerPtr->m_Running || m_WasRunning)
		{
			World::Direction dir = m_PlayerPtr->m_Direction;
			m_PlayerPos.x += ((float)(dir == World::Direction::EAST) * 5.0f) + ((float)(dir == World::Direction::WEST) * -5.0f);

			m_WasRunning = m_PlayerPtr->m_Running;
		}
	}
}

void StateRender::Overworld::update(double deltaTime)
{
	//Update transitions
	for (int i = 0; i < m_Transitions.size(); i++)
	{
		transition(i).update(deltaTime);
	}

	//Update timers
	m_Time += deltaTime;
	m_WindTimer += deltaTime;

	//Update wind and position
	updatePlayerPos();
	handleWind();

	//Update camera frustum
	camera.updateFrustum();
}

void StateRender::Overworld::shadowPass(glm::vec3 lightDir)
{
	////Shadow pass
	m_ShadowMap.bindForWriting();

	//Normal scene
	shader(StateShader::OVERWORLD_SHADOW).bind();

	//Use world renderer matrix - only works for sprite too as model matrixes are same (currently)
	glm::mat4 world = this->at(StateRen::OVERWORLD).modelMatrix;
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightView = glm::translate(lightView, -camera.getPos());

	lightView[3][0] = floor(lightView[3][0]);
	lightView[3][1] = floor(lightView[3][1]);
	lightView[3][2] = floor(lightView[3][2]);

	shader(StateShader::OVERWORLD_SHADOW).setUniform("WVP", m_ShadowMap.calcMVP(world, lightView));

	//Render from lights perspective
	m_ShadowMap.startCapture();
	texture(StateTex::OVERWORLD).bind();
	this->at(StateRen::OVERWORLD).drawPrimitives();
	texture(StateTex::OVERWORLD_POKEMON).bind();
	this->at(StateRen::OVERWORLD_POKEMON).drawPrimitives();
	texture(StateTex::OVERWORLD_SPRITE).bind();
	this->at(StateRen::OVERWORLD_SPRITE).drawPrimitives();
	m_TextureAtlas.bind();
	this->at(StateRen::OVERWORLD_MODEL).drawPrimitives();
	shader(StateShader::OVERWORLD_SHADOW).unbind();

	//Tree scene
	shader(StateShader::OVERWORLD_TREE_SHADOW).bind();
	texture(StateTex::OVERWORLD).bind();
	texture(StateTex::OVERWORLD_WIND_A).bind();
	shader(StateShader::OVERWORLD_TREE_SHADOW).setUniform("WVP", m_ShadowMap.calcMVP(world, lightView));
	shader(StateShader::OVERWORLD_TREE_SHADOW).setUniform("u_WindA", OVERWORLD_WIND_SLOT_A);
	shader(StateShader::OVERWORLD_TREE_SHADOW).setUniform("u_WindB", OVERWORLD_WIND_SLOT_B);
	shader(StateShader::OVERWORLD_TREE_SHADOW).setUniform("u_WeightA", m_WindWeightA);
	this->at(StateRen::OVERWORLD_TREE).drawPrimitives();
	shader(StateShader::OVERWORLD_TREE_SHADOW).unbind();

	m_ShadowMap.endCapture(SCREEN_WIDTH, SCREEN_HEIGHT);

	// reset viewport
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void StateRender::Overworld::lightingPass(glm::vec3 lightDir)
{
	////Lighting pass
	shader(StateShader::OVERWORLD).bind();
	m_ShadowMap.bindForReading(OVERWORLD_SHADOWS_SLOT);

	//Scale lighting color
	m_LightScaled = m_LightColor * m_LightScaleFactor;

	//Set Overworld shader uniforms
	shader(StateShader::OVERWORLD).setUniform("u_AmbLight", &m_LightScaled);
	shader(StateShader::OVERWORLD).setUniform("u_LightDir", &lightDir);
	shader(StateShader::OVERWORLD).setUniform("u_LightMVP", m_ShadowMap.getMVP());
	shader(StateShader::OVERWORLD).setUniform("u_ShadowMap", OVERWORLD_SHADOWS_SLOT);
	shader(StateShader::OVERWORLD).setUniform("u_LightsActive", m_HideShadows);
	shader(StateShader::OVERWORLD).setUniform("u_Texture", OVERWORLD_TEXTURE_SLOT);
	shader(StateShader::OVERWORLD).setUniform("u_Samples", Options::shadowSamples);

	//Sprites
	texture(StateTex::OVERWORLD_SPRITE).bind();
	glm::mat4 SpriteInvTranspModel = glm::mat4(glm::transpose(glm::inverse(this->at(StateRen::OVERWORLD_SPRITE).modelMatrix)));
	shader(StateShader::OVERWORLD).setUniform("u_InvTranspModel", &SpriteInvTranspModel);
	shader(StateShader::OVERWORLD).setUniform("u_Model", &this->at(StateRen::OVERWORLD_SPRITE).modelMatrix);
	this->at(StateRen::OVERWORLD_SPRITE).drawPrimitives();

	//Pokemon Sprite
	texture(StateTex::OVERWORLD_POKEMON).bind();
	glm::mat4 PkmInvTranspModel = glm::mat4(glm::transpose(glm::inverse(this->at(StateRen::OVERWORLD_POKEMON).modelMatrix)));
	shader(StateShader::OVERWORLD).setUniform("u_InvTranspModel", &PkmInvTranspModel);
	shader(StateShader::OVERWORLD).setUniform("u_Model", &this->at(StateRen::OVERWORLD_POKEMON).modelMatrix);
	this->at(StateRen::OVERWORLD_POKEMON).drawPrimitives();

	//Models - Make have per model inv transp later when models are used more
	m_TextureAtlas.bind();
	this->at(StateRen::OVERWORLD_MODEL).drawPrimitives();

	//World
	texture(StateTex::OVERWORLD).bind();
	glm::mat4 WorldInvTranspModel = glm::mat4(glm::transpose(glm::inverse(this->at(StateRen::OVERWORLD).modelMatrix)));
	shader(StateShader::OVERWORLD).setUniform("u_InvTranspModel", &WorldInvTranspModel);
	shader(StateShader::OVERWORLD).setUniform("u_Model", &this->at(StateRen::OVERWORLD).modelMatrix);
	this->at(StateRen::OVERWORLD).drawPrimitives();
	shader(StateShader::OVERWORLD).unbind();

	//Grass
	shader(StateShader::OVERWORLD_GRASS).bind();
	m_ShadowMap.bindForReading(OVERWORLD_SHADOWS_SLOT);

	shader(StateShader::OVERWORLD_GRASS).setUniform("u_AmbLight", &m_LightScaled);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_LightDir", &lightDir);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_LightMVP", m_ShadowMap.getMVP());
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_ShadowMap", OVERWORLD_SHADOWS_SLOT);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_LightsActive", m_HideShadows);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_Texture", OVERWORLD_TEXTURE_SLOT);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_WindA", OVERWORLD_WIND_SLOT_A);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_WindB", OVERWORLD_WIND_SLOT_B);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_WeightA", m_WindWeightA);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_PlayerPos", &m_PlayerPos);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_InvTranspModel", &WorldInvTranspModel);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_Model", &this->at(StateRen::OVERWORLD).modelMatrix);
	shader(StateShader::OVERWORLD_GRASS).setUniform("u_Samples", Options::shadowSamples/4); //Has lower samples due to many blades slowdown
	this->at(StateRen::OVERWORLD_GRASS).drawPrimitives();
	shader(StateShader::OVERWORLD_GRASS).unbind();

	//Trees
	shader(StateShader::OVERWORLD_TREE).bind();

	shader(StateShader::OVERWORLD_TREE).setUniform("u_AmbLight", &m_LightScaled);
	shader(StateShader::OVERWORLD_TREE).setUniform("u_LightDir", &lightDir);
	shader(StateShader::OVERWORLD_TREE).setUniform("u_LightMVP", m_ShadowMap.getMVP());
	shader(StateShader::OVERWORLD_TREE).setUniform("u_ShadowMap", OVERWORLD_SHADOWS_SLOT);
	shader(StateShader::OVERWORLD_TREE).setUniform("u_LightsActive", m_HideShadows);
	shader(StateShader::OVERWORLD_TREE).setUniform("u_Texture", OVERWORLD_TEXTURE_SLOT);
	shader(StateShader::OVERWORLD_TREE).setUniform("u_WindA", OVERWORLD_WIND_SLOT_A);
	shader(StateShader::OVERWORLD_TREE).setUniform("u_WindB", OVERWORLD_WIND_SLOT_B);
	shader(StateShader::OVERWORLD_TREE).setUniform("u_WeightA", m_WindWeightA);
	shader(StateShader::OVERWORLD_TREE).setUniform("u_InvTranspModel", &WorldInvTranspModel);
	shader(StateShader::OVERWORLD_TREE).setUniform("u_Model", &this->at(StateRen::OVERWORLD_TREE).modelMatrix);
	shader(StateShader::OVERWORLD_TREE).setUniform("u_Samples", Options::shadowSamples);
	this->at(StateRen::OVERWORLD_TREE).drawPrimitives();
	shader(StateShader::OVERWORLD_TREE).unbind();
}

void StateRender::Overworld::handleTransition()
{
	//Handle fading
	if (transition(StateTrans::OVERWORLD_FADE_IN).isEnded())
	{

	}
	if (transition(StateTrans::OVERWORLD_FADE_OUT).isEnded())
	{
		readyToShow = false;
	}
}

void StateRender::Overworld::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Handle fading
	handleTransition();
	if (!readyToShow)
	{
		return;
	}

	//Send cam uniforms
	shader(StateShader::OVERWORLD).bind();
	camera.sendCameraUniforms(shader(StateShader::OVERWORLD));
	shader(StateShader::OVERWORLD_GRASS).bind();
	camera.sendCameraUniforms(shader(StateShader::OVERWORLD_GRASS));
	shader(StateShader::OVERWORLD_TREE).bind();
	camera.sendCameraUniforms(shader(StateShader::OVERWORLD_TREE));

	glm::vec3 lightDir = glm::normalize(m_LightDir);

	////Shadow Pass
	if (!m_HideShadows)
	{
		shadowPass(lightDir);
	}

	////Lighting pass
	lightingPass(lightDir);

	//Transition
	transition(StateTrans::OVERWORLD_BATTLE).render(shader(StateShader::OVERWORLD_BATTLE));
	transition(StateTrans::OVERWORLD_FADE_OUT).render(shader(StateShader::OVERWORLD_FADE_OUT));
	transition(StateTrans::OVERWORLD_FADE_IN).render(shader(StateShader::OVERWORLD_FADE_IN));
}

//battle
void StateRender::Battle::initialiseRenderer(unsigned int width, unsigned int height)
{
	//Renderer setup
	camera = SGRender::Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

	//Camera
	camera.setProjection(glm::perspective(glm::radians(45.0f), (float)((float)width / (float)height), 0.1f, 4000.0f));
	camera.setFOV(45.0f);
	camera.moveUp((float)width	/  19.2f);
	camera.moveZ((float)width	/ -22.2f);
	camera.moveX((float)width	/ -38.4f);
	camera.panYDegrees(15.0f);

	SCREEN_HEIGHT = height; SCREEN_WIDTH = width;
}

void StateRender::Battle::loadRendererData()
{
	using namespace SGRender;
	//Shader
	sceneShader.create(
		"res/shaders/vert/Default_T_Shader.vert",
		"res/shaders/frag/Default_T_Shader.frag"
	);

	renderers.resize((int)StateRen::BATTLE_COUNT);

	//World Renderer
	this->at(StateRen::BATTLE_PLATFORM).setLayout<float>(3, 2);
	this->at(StateRen::BATTLE_PLATFORM).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::BATTLE_PLATFORM).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera, sizeof(TVertex));

	//Background Renderer
	this->at(StateRen::BATTLE_BACKGROUND).setLayout<float>(3, 2);
	this->at(StateRen::BATTLE_BACKGROUND).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::BATTLE_BACKGROUND).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera, sizeof(TVertex));

	//Pokemon renderer
	this->at(StateRen::BATTLE_POKEMONA).setLayout<float>(3, 2);
	this->at(StateRen::BATTLE_POKEMONA).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::BATTLE_POKEMONA).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera, sizeof(TVertex));
	this->at(StateRen::BATTLE_POKEMONB).setLayout<float>(3, 2);
	this->at(StateRen::BATTLE_POKEMONB).setDrawingMode(GL_TRIANGLES);
	this->at(StateRen::BATTLE_POKEMONB).generate((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, &camera, sizeof(TVertex));

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

void StateRender::Battle::purgeData()
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

void StateRender::Battle::loadPokemonTextureA(std::string name)
{
	texture(StateTex::BATTLE_POKEMONA).loadTexture(BACK_TEX_PATH+name+PNG_EXT);
	texture(StateTex::BATTLE_POKEMONA).generateTexture(BATTLE_TEXTURE_SLOT);
	texture(StateTex::BATTLE_POKEMONA).bind();
	texture(StateTex::BATTLE_POKEMONA).clearBuffer();
}

void StateRender::Battle::loadPokemonTextureB(std::string name)
{
	texture(StateTex::BATTLE_POKEMONB).loadTexture(FRONT_TEX_PATH+name+PNG_EXT);
	texture(StateTex::BATTLE_POKEMONB).generateTexture(BATTLE_TEXTURE_SLOT);
	texture(StateTex::BATTLE_POKEMONB).bind();
	texture(StateTex::BATTLE_POKEMONB).clearBuffer();
}

void StateRender::Battle::bufferRenderData()
{
	for (int i = 0; i < renderers.size(); i++)
	{
		renderers[i].bufferVideoData();
	}
}

void StateRender::Battle::draw()
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
		sceneShader.setUniform("u_Model", &renderers[i].modelMatrix);
		renderers[i].drawPrimitives();
	}

	sceneShader.unbind();
}
