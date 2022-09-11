#include "game/states/Overworld.h"

void Overworld::init(int width, int height, PlayerData* data, World::LevelID levelEntry, FontContainer* fonts, FlagArray* flags, GameInput* input, SGSound::System* system) {
    //Width and height
    m_Width = width; m_Height = height;

    //Flags
    m_Flags = flags;

    //Input
    m_Input = input;

    //Data
    m_Data = data;

    //Sound
    m_System = system;

    //Fonts - TODO make load in loadRequiredData()
    m_Fonts = fonts;
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 20);
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 45);
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 70);

    m_Renderer.initialiseRenderer(width, height, &m_ObjManager);

    //Init levels and scripts
    m_Levels.InitialiseLevels(&m_ObjManager, &m_Renderer, m_Data, &m_TextBuff, m_Input);
    std::function<void(World::LevelID)> ld = std::bind(&World::LevelContainer::SignalLoadLevel, &m_Levels, std::placeholders::_1);
    std::function<void(World::LevelID)> uld = std::bind(&World::LevelContainer::SignalUnloadLevel, &m_Levels, std::placeholders::_1);
    OverworldScript::init(m_Data, m_Input, ld, uld);

    std::shared_ptr<GameGUI::HUD> hud(new GameGUI::HUD(width, height, 0, 0));
    m_HUD.setBase(hud);
    std::shared_ptr<GameGUI::TextDisplay> tb(new GameGUI::TextDisplay(m_TextBuff.buffer.line1, m_TextBuff.buffer.line2));
    tb->setFontContainer(m_Fonts);
    tb->m_Width = m_Width / 1.3f; tb->m_Height = 300.0f;
    tb->setNest(1);
    tb->m_XPos = 0.0f + (m_Width / 2 - m_Width / 2.6f);
    tb->m_YPos = m_Height - 375.0f;
    m_HUD.addElement(tb);
    tb->setNest(1);
    m_HUD.showNest(1, false);

    //TODO - make debug view more workable
    std::shared_ptr<GameGUI::TextBox> tex(new GameGUI::TextBox(m_RenderTime));
    std::shared_ptr<GameGUI::TextBox> tex2(new GameGUI::TextBox(m_UpdateTime));
    std::shared_ptr<GameGUI::TextBox> tex3(new GameGUI::TextBox(m_CurrentLevelStr));
    std::shared_ptr<GameGUI::TextBox> tex4(new GameGUI::TextBox(m_CurrentTileStr));
    std::shared_ptr<GameGUI::TextBox> tex5(new GameGUI::TextBox(m_ObjectCountStr));

    tex->setNest(0);
    tex2->setNest(0);
    tex3->setNest(0);
    tex4->setNest(0);
    tex5->setNest(0);
    
    m_HUD.addElement(tex);
    m_HUD.addElement(tex2);
    m_HUD.addElement(tex3);
    m_HUD.addElement(tex4);
    m_HUD.addElement(tex5);

    std::shared_ptr<OverworldMenu> menu(new OverworldMenu());
    menu->m_Width = 300.0f; menu->m_Height = m_Height - 20.0f;
    menu->setNest(2);
    menu->m_XPos = m_Width - 320.0f;
    menu->m_YPos = 10.0f;
    menu->setFontContainer(m_Fonts);
    menu->setPlayerData(m_Data);
    menu->linkSoundSystem(m_System);

    m_HUD.addElement(menu);

    //Init pkm
    //Pkm test
    m_Pool = MtLib::ThreadPool::Fetch();

    PokemonDataBank::loadData(PkmDataType::BASE_STATS);
    PokemonDataBank::loadData(PkmDataType::MOVE_INFO);
    PokemonDataBank::loadData(PkmDataType::POKEMON_TYPES);
    PokemonDataBank::loadData(PkmDataType::SPECIES_INFO);

    m_Data->playerParty[0].id = 1;
    GeneratePokemon(m_Data->playerParty[0].id, m_Data->playerParty[0]);
    SetPkmStatsFromLevel(m_Data->playerParty[0]);
    m_Data->playerParty[1].id = 9;
    GeneratePokemon(m_Data->playerParty[1].id, m_Data->playerParty[1]);
    SetPkmStatsFromLevel(m_Data->playerParty[1]);
    enemy[0].id = 467;
    GeneratePokemon(enemy[0].id, enemy[0]);
    SetPkmStatsFromLevel(enemy[0]);

    m_Data->playerParty[0].nickname = PokemonDataBank::GetPokemonName(m_Data->playerParty[0].id);
    m_Data->playerParty[1].nickname = PokemonDataBank::GetPokemonName(m_Data->playerParty[1].id);
    enemy[0].nickname = PokemonDataBank::GetPokemonName(467);
    m_Data->playerParty[0].moves[0].id = 33;
    m_Data->playerParty[0].moves[1].id = 77;
    m_Data->playerParty[0].moves[2].id = 22;
    m_Data->playerParty[1].moves[0].id = 91;
    m_Data->playerParty[1].moves[1].id = 130;
    m_Data->playerParty[1].moves[2].id = 399;
    m_Data->playerParty[1].moves[2].id = 58;
    enemy[0].moves[0].id = 7;
    PokemonDataBank::LoadPokemonMoves(m_Data->playerParty[0]);
    PokemonDataBank::LoadPokemonMoves(m_Data->playerParty[1]);
    PokemonDataBank::LoadPokemonMoves(enemy[0]);

    m_Pool->Run(PokemonDataBank::unloadData, PkmDataType::SPECIES_INFO);
    m_Pool->Run(PokemonDataBank::unloadData, PkmDataType::BASE_STATS);
    m_Pool->Run(PokemonDataBank::unloadData, PkmDataType::MOVE_INFO);
    m_Pool->Run(PokemonDataBank::unloadData, PkmDataType::POKEMON_TYPES);

    EngineLog("Overworld loaded: ", (int)m_CurrentLevel);
}

void Overworld::setBattleFunction(std::function<void(Party*, Party*)> battle)
{
    //Battle func
    m_BattleEnable = battle; m_Renderer.m_StateToBattle = std::bind(&Overworld::startBattle, this);
}

void Overworld::loadRequiredData() {
    m_Renderer.loadRendererData();
    loadObjectData();

    //Init levels
    m_Levels.InitialiseGlobalObjects();
    m_Levels.SignalLoadLevel(m_CurrentLevel);
    m_Renderer.generateAtlas();
    
    m_StartBattle = false;
    m_DataLoaded = true;
}

void Overworld::purgeRequiredData() {
    m_ObjManager.reset();
    m_Renderer.purgeData();
    m_Levels.UnloadAll();
    m_DataLoaded = false;
}

void Overworld::createGroups()
{
    using namespace StateRender; using namespace SGObject;

    //Create groups
    //Add component groups
    std::shared_ptr<UpdateCompGroup<Ov_Sprite::TilePosition>> tileGroup(new UpdateCompGroup<Ov_Sprite::TilePosition>());
    std::shared_ptr<UpdateCompGroup<Ov_Sprite::SpriteMap>> mapGroup(new UpdateCompGroup<Ov_Sprite::SpriteMap>());
    std::shared_ptr<UpdateCompGroup<SpriteAnim<SGRender::NTVertex, Norm_Tex_Quad>>> animGroup(new UpdateCompGroup<SpriteAnim<SGRender::NTVertex, Norm_Tex_Quad>>());
    std::shared_ptr<UpdateCompGroup<Ov_Sprite::UpdateAnimationRunning>> runGroup(new UpdateCompGroup<Ov_Sprite::UpdateAnimationRunning>());
    std::shared_ptr<UpdateCompGroup<Ov_Sprite::UpdateAnimationFacing>> faceGroup(new UpdateCompGroup<Ov_Sprite::UpdateAnimationFacing>());
    std::shared_ptr<UpdateCompGroup<Ov_Sprite::UpdateAnimationWalking>> walkGroup(new UpdateCompGroup<Ov_Sprite::UpdateAnimationWalking>());
    std::shared_ptr<UpdateCompGroup<NPC_RandWalk>> randWGroup(new UpdateCompGroup<NPC_RandWalk>());
    std::shared_ptr<UpdateCompGroup<WarpTileUpdateComponent>> warpGroup(new UpdateCompGroup<WarpTileUpdateComponent>());
    std::shared_ptr<RenderCompGroup<ModelRender>> modRenGroup(new RenderCompGroup<ModelRender>());
    std::shared_ptr<RenderCompGroup<Ov_Sprite::SpriteRender>> spriteGroup(new RenderCompGroup<Ov_Sprite::SpriteRender>());

    //Add component groups
    m_ObjManager.pushRenderGroup(spriteGroup, "SpriteRender");
    m_ObjManager.pushRenderGroup(modRenGroup, "ModelRender");
    m_ObjManager.pushUpdateGroup(tileGroup, "TilePosition");
    m_ObjManager.pushUpdateGroup(mapGroup, "SpriteMap");
    m_ObjManager.pushUpdateGroup(animGroup, "SpriteAnim");
    m_ObjManager.pushUpdateGroup(runGroup, "UpdateRunning");
    m_ObjManager.pushUpdateGroup(faceGroup, "UpdateFacing");
    m_ObjManager.pushUpdateGroup(walkGroup, "UpdateWalking");
    m_ObjManager.pushUpdateGroup(randWGroup, "RandWalk");
    m_ObjManager.pushUpdateGroup(warpGroup, "WarpTile");

    //Store
    m_TileGroup = tileGroup.get();
    m_MapGroup = mapGroup.get();
    m_AnimGroup = animGroup.get();
    m_RunGroup = runGroup.get();
    m_FaceGroup = faceGroup.get();
    m_WalkGroup = walkGroup.get();
    m_RandWGroup = randWGroup.get();
    m_WarpGroup = warpGroup.get();
    m_ModRenGroup = modRenGroup.get();
    m_SpriteGroup = spriteGroup.get();
}

void Overworld::loadObjectData()
{
    using namespace StateRender;

    //Create bulk update/render groups
    createGroups();

    //Load level data
    m_Levels.BuildFirstLevel(m_CurrentLevel);

    //Add player and link pos to lighting
    Ov_Sprite::SpriteData dataPlayer = { m_Data->tile,  m_Data->height, m_Data->id, {0, 4} };
    m_PlayerPtr = Ov_ObjCreation::BuildRunningSprite(dataPlayer, m_Renderer.tilemap(StateTileMap::OVERWORLD_SPRITE), m_SpriteGroup, m_MapGroup, m_RunGroup, &m_Renderer[StateRen::OVERWORLD_SPRITE]);
    m_SpriteGroup->addComponent(&m_PlayerPtr->m_RenderComps, &m_PlayerPtr->m_Sprite, &m_Renderer[StateRen::OVERWORLD_SPRITE]);
    m_PlayerPtr->m_LastPermissionPtr = World::GetTilePermission(m_PlayerPtr->m_CurrentLevel, m_PlayerPtr->m_Tile, m_PlayerPtr->m_WorldLevel);
    m_PlayerPtr->setTag((uint16_t)ObjectType::RunningSprite);

    std::shared_ptr<PlayerMove> walk(new PlayerMove(&m_PlayerPtr->m_CurrentLevel, &m_PlayerPtr->m_XPos, &m_PlayerPtr->m_ZPos, &m_PlayerPtr->m_Tile));
    std::shared_ptr<PlayerCameraLock> spCam(new PlayerCameraLock(&m_PlayerPtr->m_XPos, &m_PlayerPtr->m_YPos, &m_PlayerPtr->m_ZPos, &m_Renderer.camera));
    std::shared_ptr<PlayerEncounter> encounter(new PlayerEncounter(&m_PlayerPtr->m_LastPermission, &m_PlayerPtr->m_Tile, &m_StartBattle));
    std::shared_ptr<UpdateGlobalLevel> globLev(new UpdateGlobalLevel(&m_CurrentLevel, &m_PlayerPtr->m_CurrentLevel));
    walk->linkInput(m_Input);
    walk->setSpriteData(m_PlayerPtr);
    m_LocationX = &m_PlayerPtr->m_Tile.x;
    m_LocationZ = &m_PlayerPtr->m_Tile.z;

    m_Renderer.linkPlayerPtr(m_PlayerPtr);

    //Add companion test
    //OvSpr_SpriteData dataCompanion = { { m_PlayerPtr->m_Tile.x - 1, m_PlayerPtr->m_Tile.z}, World::WorldHeight::F0, m_Data->id, { 0, 0 } };
    //std::shared_ptr<OvSpr_RunningSprite> companionPtr = Ov_ObjCreation::BuildRunningSprite(dataCompanion, m_Renderer.pokemonTileMap, spriteGroup.get(), mapGroup.get(), &m_Renderer.pokemonRenderer);
    //companionPtr->setTag((uint16_t)ObjectType::RunningSprite);
    //spriteGroup->addComponent(&companionPtr->m_RenderComps, &companionPtr->m_Sprite, &m_Renderer.pokemonRenderer);
    //companionPtr->m_LastPermissionPtr = World::GetTilePermission(companionPtr->m_CurrentLevel, companionPtr->m_Tile, companionPtr->m_WorldLevel);
    //std::shared_ptr<PokemonFollow> follow(new PokemonFollow(&companionPtr->m_CurrentLevel, &companionPtr->m_XPos, &companionPtr->m_ZPos, &companionPtr->m_Tile));
    //ScriptParse::ScriptWrapper scriptWrap = ScriptParse::ParseScriptFromText("blank.sgs");
    //std::shared_ptr<OverworldScript> script(new OverworldScript(scriptWrap.script, scriptWrap.size, companionPtr));
    //script->linkText(&m_TextBuff);
    //follow->linkScript(script.get());
    //follow->linkInput(m_Input);
    //std::shared_ptr<PokemonAnimation> pkmAnim(new PokemonAnimation(companionPtr.get()));

    //Add pointer test
    //OvSpr_SpriteData ptrData = { { m_PlayerPtr->m_Tile.x - 2, m_PlayerPtr->m_Tile.z}, World::WorldHeight::F0, m_Data->id, { 0, 0 } };
    //std::shared_ptr<OvSpr_Sprite> ptrSprite = Ov_ObjCreation::BuildSprite(ptrData, m_Renderer.worldTileMap, spriteGroup.get(), &m_Renderer.worldRenderer, false);
    //AxialRotate<NormalTextureVertex>(&ptrSprite->m_Sprite, { ptrSprite->m_XPos + World::TILE_SIZE / 2.0f,  ptrSprite->m_YPos, ptrSprite->m_ZPos }, -45.0f, GetVerticesCount(Shape::QUAD), Axis::X);
    //Translate<NormalTextureVertex>(&ptrSprite->m_Sprite, 0.0f, 1.0f, World::TILE_SIZE / 2.0f, Shape::QUAD);
    //SpriteAnim<NormalTextureVertex, Norm_Tex_Quad> anim = SpriteAnim<NormalTextureVertex, Norm_Tex_Quad>(8, 3);
    //TileUV uv1 = m_Renderer.worldTileMap.uvTile(0, 6);
    //TileUV uv2 = m_Renderer.worldTileMap.uvTile(0, 7);
    //TileUV uv3 = m_Renderer.worldTileMap.uvTile(0, 8);
    //anim.setFrame(0, uv1); anim.setFrame(1, uv2); anim.setFrame(2, uv3);
    //anim.linkSprite(&ptrSprite->m_Sprite, &m_AnimatePointer);
    //anim.loop = true;
    //animGroup->addExistingComponent(&companionPtr->m_UpdateComps, anim);
    //ptrSprite->messageAllRender((int)Message::DEACTIVATE);
    //tileGroup->addComponent(&ptrSprite->m_UpdateComps, &ptrSprite->m_CurrentLevel, &ptrSprite->m_XPos, &ptrSprite->m_ZPos, &ptrSprite->m_Tile);

    //Link sprites to pkm follow
    //follow->linkSprites(m_PlayerPtr.get(), companionPtr.get(), ptrSprite.get());

    //Player is always first object pushed on overworld load - makes searching easier
    m_ObjManager.pushUpdateHeap(walk, &m_PlayerPtr->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(globLev, &m_PlayerPtr->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(encounter, &m_PlayerPtr->m_UpdateComps);
    //m_ObjManager.pushUpdateHeap(follow, &companionPtr->m_UpdateComps);
    //m_ObjManager.pushUpdateHeap(script, &companionPtr->m_UpdateComps);
    //m_ObjManager.pushUpdateHeap(pkmAnim, &companionPtr->m_UpdateComps);
    m_ObjManager.pushRenderHeap(spCam, &m_PlayerPtr->m_RenderComps);
    m_ObjManager.pushGameObject(m_PlayerPtr, "Player");
    //m_ObjManager.pushGameObject(companionPtr, "Companion");
    //m_ObjManager.pushGameObject(ptrSprite, "Pointer");
}

void Overworld::update(double deltaTime, double time) {
    using namespace StateRender;
    if (!m_Sample)
    {
        m_SampleTime += deltaTime;
        if (m_SampleTime > 0.357f)
        {
            
            m_Sample = true;
            m_SampleTime = 0.0f;
        }
    }
    else
    {
        m_Sample = false;
    }
    
    //Start timer
    auto ts = EngineTimer::StartTimer();

    m_Input->update(deltaTime);

    //update objects
    m_ObjManager.update(deltaTime);

    //update levels
    if (!m_Renderer.transition(StateTrans::OVERWORLD_FADE_OUT).isStarted())
    {
        m_Levels.ChangeLevel();
    }

    //update renderer
    m_Renderer.update(deltaTime);

    //test
    m_CurrentLevelStr = "Current Level: " + std::to_string((int)m_CurrentLevel);
    m_CurrentTileStr = "Current Tile: " + std::to_string((int)*m_LocationX) + ", " + std::to_string((int)*m_LocationZ);
    m_ObjectCountStr = "Objects: " + std::to_string(m_ObjManager.getObjectCount());
   
    //Start battle
    if (m_StartBattle && (m_PlayerPtr->m_Walking || m_PlayerPtr->m_Running))
    {
        m_PlayerPtr->m_Busy = true;
    }
    else if (m_StartBattle && !(m_PlayerPtr->m_Walking || m_PlayerPtr->m_Running) && m_PlayerPtr->m_Busy)
    {
        //Store player data and start
        m_Data->tile = m_PlayerPtr->m_Tile;
        m_Data->height = m_PlayerPtr->m_WorldLevel;

        //Start transition
        m_Renderer.transition(StateTrans::OVERWORLD_BATTLE).start();
    }
    if (m_StartBattle && m_Renderer.transition(StateTrans::OVERWORLD_BATTLE).isStarted())
    {
        if (m_Renderer.transition(StateTrans::OVERWORLD_BATTLE).isEnded())
        {
            //Await any pending tasks then start battle
            m_Pool->Wait();
            this->startBattle();
        }
    }

    //End timer
    double timeTaken = EngineTimer::EndTimer(ts) * 1000.0;
    
    if (m_Sample)
    {
        m_UpdateTime = "Time to update: " + std::to_string(timeTaken) + "ms";
    }
}

void Overworld::render() {

    //Start timer
    auto ts = EngineTimer::StartTimer();

    //Renders
    m_Levels.render();
    m_ObjManager.render();
    
    //Draw scene
    m_Renderer.bufferRenderData();
    m_Renderer.draw();

    //Display timer
    double timeTaken = EngineTimer::EndTimer(ts) * 1000.0;
    if (m_Sample)
    {
        m_RenderTime = "Time to render: " + std::to_string(timeTaken) + "ms";
    }

    //IMGUI Test
    GameGUI::StartFrame();
    m_HUD.showNest(0, m_ShowDebug);
    m_HUD.showNest(1, m_TextBuff.showTextBox);
    m_HUD.showNest(2, m_ShowMenu);
    m_HUD.render();
    GameGUI::EndFrame();
}

void Overworld::handleInput(int key, int scancode, int action, int mods) 
{
    using namespace StateRender;
    
    m_Input->handleInput(key, scancode, action, mods);

    //Debug
    if (key == GLFW_KEY_Y)
    {
        if (action == GLFW_PRESS)
        {
            m_Renderer[StateRen::OVERWORLD].setDrawingMode(GL_LINES);
        }
    }
    if (key == GLFW_KEY_U)
    {
        if (action == GLFW_PRESS)
        {
            m_Renderer[StateRen::OVERWORLD].setDrawingMode(GL_TRIANGLES);
        }
    }
    if (key == GLFW_KEY_L)
    {
        if (action == GLFW_PRESS)
        {
            m_Renderer.showShadows(!m_Renderer.showingShadows());
        }
    }
    if (key == GLFW_KEY_F3)
    {
        if (action == GLFW_PRESS)
        {
            m_ShowDebug = !m_ShowDebug;
        }
    }
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_BACKSPACE)
    {
        if (action == GLFW_PRESS)
        {
            if (!m_ShowMenu && !(m_PlayerPtr->m_Walking || m_PlayerPtr->m_Running))
            {
                m_ShowMenu = true;
                m_PlayerPtr->m_Busy = m_ShowMenu;
            }
            else if (m_ShowMenu)
            {
                m_ShowMenu = false;
                m_PlayerPtr->m_Busy = m_ShowMenu;
            }
        }
    }
}

void Overworld::startBattle()
{
    this->setActive(false);
    m_BattleEnable(&m_Data->playerParty, &enemy);
}