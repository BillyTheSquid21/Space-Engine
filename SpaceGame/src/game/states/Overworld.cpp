#include "game/states/Overworld.h"

void Overworld::init(int width, int height, PlayerData* data, World::LevelID levelEntry, FontContainer* fonts, FlagArray* flags, GameInput* input) {
    //Width and height
    m_Width = width; m_Height = height;

    //Flags
    m_Flags = flags;

    //Input
    m_Input = input;

    //Data
    m_Data = data;

    //Fonts - TODO make load in loadRequiredData()
    m_Fonts = fonts;
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 20);
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 45);
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 70);

    m_Renderer.initialiseRenderer(width, height);

    //test level
    m_Levels.InitialiseLevels(&m_ObjManager, &m_Renderer, m_Flags, &m_TextBuff, m_Input);

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

    m_HUD.addElement(menu);

    //Init pkm
    //Pkm test
    MtLib::ThreadPool* pool = MtLib::ThreadPool::Fetch();

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
    enemy[0].id = 6;
    GeneratePokemon(enemy[0].id, enemy[0]);
    SetPkmStatsFromLevel(enemy[0]);

    m_Data->playerParty[0].nickname = PokemonDataBank::GetPokemonName(m_Data->playerParty[0].id);
    m_Data->playerParty[1].nickname = PokemonDataBank::GetPokemonName(m_Data->playerParty[1].id);
    enemy[0].nickname = PokemonDataBank::GetPokemonName(6);
    m_Data->playerParty[0].moves[0].id = 33;
    m_Data->playerParty[0].moves[1].id = 77;
    m_Data->playerParty[0].moves[2].id = 22;
    m_Data->playerParty[1].moves[0].id = 61;
    m_Data->playerParty[1].moves[1].id = 130;
    m_Data->playerParty[1].moves[2].id = 399;
    m_Data->playerParty[1].moves[2].id = 58;
    m_Data->playerParty[1].moves[0].id = 61;
    enemy[0].moves[0].id = 7;
    PokemonDataBank::LoadPokemonMoves(m_Data->playerParty[0]);
    PokemonDataBank::LoadPokemonMoves(m_Data->playerParty[1]);
    PokemonDataBank::LoadPokemonMoves(enemy[0]);

    pool->Run(PokemonDataBank::unloadData, PkmDataType::SPECIES_INFO);
    pool->Run(PokemonDataBank::unloadData, PkmDataType::BASE_STATS);
    pool->Run(PokemonDataBank::unloadData, PkmDataType::MOVE_INFO);
    pool->Run(PokemonDataBank::unloadData, PkmDataType::POKEMON_TYPES);

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
    m_Levels.LoadLevel(m_CurrentLevel);
    m_Renderer.generateAtlas();
    
    m_StartBattle = false;
    m_DataLoaded = true;
}

void Overworld::purgeRequiredData() {
    m_Renderer.purgeData();
    m_ObjManager.reset();
    m_Levels.UnloadAll();
    m_DataLoaded = false;
}

void Overworld::loadObjectData()
{
    //Create groups
    //Add component groups
    std::shared_ptr<RenderComponentGroup<SpriteRender>> spriteGroup(new RenderComponentGroup<SpriteRender>());
    std::shared_ptr<UpdateComponentGroup<TilePosition>> tileGroup(new UpdateComponentGroup<TilePosition>());
    std::shared_ptr<UpdateComponentGroup<SpriteMap>> mapGroup(new UpdateComponentGroup<SpriteMap>());
    std::shared_ptr<UpdateComponentGroup<SpriteAnim<TextureVertex, Tex_Quad>>> animGroup(new UpdateComponentGroup<SpriteAnim<TextureVertex, Tex_Quad>>());
    std::shared_ptr<UpdateComponentGroup<UpdateAnimationRunning>> runGroup(new UpdateComponentGroup<UpdateAnimationRunning>());
    std::shared_ptr<UpdateComponentGroup<UpdateAnimationFacing>> faceGroup(new UpdateComponentGroup<UpdateAnimationFacing>());
    std::shared_ptr<UpdateComponentGroup<UpdateAnimationWalking>> walkGroup(new UpdateComponentGroup<UpdateAnimationWalking>());
    std::shared_ptr<UpdateComponentGroup<NPC_RandWalk>> randWGroup(new UpdateComponentGroup<NPC_RandWalk>());
    std::shared_ptr<UpdateComponentGroup<WarpTileUpdateComponent>> warpGroup(new UpdateComponentGroup<WarpTileUpdateComponent>());
    std::shared_ptr<RenderComponentGroup<ModelAtlasUpdate>> modAtlasGroup(new RenderComponentGroup<ModelAtlasUpdate>());
    std::shared_ptr<RenderComponentGroup<ModelRender>> modRenGroup(new RenderComponentGroup<ModelRender>());

    //Add component groups
    m_ObjManager.pushRenderGroup(spriteGroup, "SpriteRender");
    m_ObjManager.pushRenderGroup(modAtlasGroup, "ModelAtlas");
    m_ObjManager.pushRenderGroup(modRenGroup, "ModelRender");
    m_ObjManager.pushUpdateGroup(tileGroup, "TilePosition");
    m_ObjManager.pushUpdateGroup(mapGroup, "SpriteMap");
    m_ObjManager.pushUpdateGroup(animGroup, "SpriteAnim");
    m_ObjManager.pushUpdateGroup(runGroup, "RunMap");
    m_ObjManager.pushUpdateGroup(faceGroup, "UpdateFacing");
    m_ObjManager.pushUpdateGroup(walkGroup, "UpdateWalking");
    m_ObjManager.pushUpdateGroup(randWGroup, "RandWalk");
    m_ObjManager.pushUpdateGroup(warpGroup, "WarpTile");

    //Load level data
    m_Levels.BuildFirstLevel(m_CurrentLevel);

    //Add player and link pos to lighting
    OvSpr_SpriteData dataPlayer = { m_Data->tile,  World::WorldHeight::F0, m_Data->id, {0, 4} };
    m_PlayerPtr = Ov_ObjCreation::BuildRunningSprite(dataPlayer, m_Renderer.spriteTileMap, spriteGroup.get(), mapGroup.get(), runGroup.get(), &m_Renderer.spriteRenderer);
    spriteGroup->addComponent(&m_PlayerPtr->m_RenderComps, &m_PlayerPtr->m_Sprite, &m_Renderer.spriteRenderer);
    m_PlayerPtr->m_LastPermissionPtr = World::GetTilePermission(m_PlayerPtr->m_CurrentLevel, m_PlayerPtr->m_Tile, m_PlayerPtr->m_WorldLevel);

    std::shared_ptr<PlayerMove> walk(new PlayerMove(&m_PlayerPtr->m_CurrentLevel, &m_PlayerPtr->m_XPos, &m_PlayerPtr->m_ZPos, &m_PlayerPtr->m_Tile.x, &m_PlayerPtr->m_Tile.z));
    std::shared_ptr<PlayerCameraLock> spCam(new PlayerCameraLock(&m_PlayerPtr->m_XPos, &m_PlayerPtr->m_YPos, &m_PlayerPtr->m_ZPos, &m_Renderer.camera));
    std::shared_ptr<PlayerEncounter> encounter(new PlayerEncounter(&m_PlayerPtr->m_LastPermission, &m_PlayerPtr->m_Tile, &m_StartBattle));
    std::shared_ptr<UpdateGlobalLevel> globLev(new UpdateGlobalLevel(&m_CurrentLevel, &m_PlayerPtr->m_CurrentLevel));
    walk->setInput(m_Input);
    walk->setSpriteData(m_PlayerPtr);
    m_LocationX = &m_PlayerPtr->m_Tile.x;
    m_LocationZ = &m_PlayerPtr->m_Tile.z;

    //Player is always first object pushed on overworld load - makes searching easier
    m_ObjManager.pushUpdateHeap(walk, &m_PlayerPtr->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(globLev, &m_PlayerPtr->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(encounter, &m_PlayerPtr->m_UpdateComps);
    m_ObjManager.pushRenderHeap(spCam, &m_PlayerPtr->m_RenderComps);
    m_ObjManager.pushGameObject(m_PlayerPtr, "Player");
}

void Overworld::update(double deltaTime, double time) {
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

        //Start transition
        m_Renderer.battleTransition.start();
    }
    if (m_StartBattle && m_Renderer.battleTransition.isStarted())
    {
        if (m_Renderer.battleTransition.isEnded())
        {
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

    //If new objects created, ensure models are mapped correctly
    m_Renderer.ensureModelMapping(m_ObjManager.getObjectCount());

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

void Overworld::handleInput(int key, int scancode, int action, int mods) {
     m_Input->handleInput(key, scancode, action, mods);

    //Debug
    if (key == GLFW_KEY_Y)
    {
        if (action == GLFW_PRESS)
        {
            m_Renderer.worldRenderer.setDrawingMode(GL_LINES);
        }
    }
    if (key == GLFW_KEY_U)
    {
        if (action == GLFW_PRESS)
        {
            m_Renderer.worldRenderer.setDrawingMode(GL_TRIANGLES);
        }
    }
    if (key == GLFW_KEY_L)
    {
        if (action == GLFW_PRESS)
        {
            m_Renderer.lightScene = !m_Renderer.lightScene;
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