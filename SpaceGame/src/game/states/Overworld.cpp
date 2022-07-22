#include "game/states/Overworld.h"

void Overworld::init(int width, int height, World::LevelID levelEntry, FontContainer* fonts, FlagArray* flags, GameInput* input, std::function<void(bool)> battle) {
    //Width and height
    m_Width = width; m_Height = height;

    //Battle func
    m_StartBattle = battle; m_Renderer.m_StateToBattle = std::bind(&Overworld::startBattle, this);

    //Flags
    m_Flags = flags;

    //Input
    m_Input = input;

    //Fonts - TODO make load in loadRequiredData()
    m_Fonts = fonts;
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 20);
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 70);

    m_Renderer.initialiseRenderer(width, height);

    //test level
    m_Levels.InitialiseLevels(&m_ObjManager, &m_Renderer, m_Flags, &m_TextBuff, m_Input);

    std::shared_ptr<GameGUI::GameTextBox> tb(new GameGUI::GameTextBox(m_Width / 1.3f, 300.0f, 0.0f + (m_Width / 2 - m_Width / 2.6f), m_Height - 375.0f, m_TextBuff.buffer.line1, m_TextBuff.buffer.line2));
    tb->setFontContainer(m_Fonts);
    m_TextBoxGUI.setBase(tb);

    //TODO - make debug view more workable
    std::shared_ptr<GameGUI::DebugPanel> dp(new GameGUI::DebugPanel(200.0f, 225.0f, 0.0f, 0.0f));
    std::shared_ptr<GameGUI::Divider> div1(new GameGUI::Divider());
    std::shared_ptr<GameGUI::Divider> div2(new GameGUI::Divider());
    std::shared_ptr<GameGUI::Divider> div3(new GameGUI::Divider());
    std::shared_ptr<GameGUI::TextBox> tex(new GameGUI::TextBox(m_RenderTime));
    std::shared_ptr<GameGUI::TextBox> tex2(new GameGUI::TextBox(m_UpdateTime));
    std::shared_ptr<GameGUI::TextBox> tex3(new GameGUI::TextBox(m_CurrentLevelStr));
    std::shared_ptr<GameGUI::TextBox> tex4(new GameGUI::TextBox(m_CurrentTileStr));
    std::shared_ptr<GameGUI::TextBox> tex5(new GameGUI::TextBox(m_ObjectCountStr));

    div1->m_FillY = false; div1->m_FillX = true;
    div1->m_Height = 72.0f;

    div2->m_FillY = false; div2->m_FillX = true;
    div2->m_Height = 72.0f;
    div2->setNest(1);

    div3->m_FillY = false; div3->m_FillX = true;
    div3->m_Height = 42.0f;
    div3->setNest(2);

    tex3->setNest(1);
    tex4->setNest(1);
    tex5->setNest(2);
    
    m_DebugGUI.setBase(dp);
    m_DebugGUI.addElement(div1);
    m_DebugGUI.addElement(tex);
    m_DebugGUI.addElement(tex2);
    m_DebugGUI.addElement(div2);
    m_DebugGUI.addElement(tex3);
    m_DebugGUI.addElement(tex4);
    m_DebugGUI.addElement(div3);
    m_DebugGUI.addElement(tex5);

    EngineLog("Overworld loaded: ", (int)m_CurrentLevel);
}

void Overworld::loadRequiredData() {
    m_Renderer.loadRendererData();

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
    OvSpr_SpriteData dataPlayer = { {8, 3},  World::WorldHeight::F0, World::LevelID::LEVEL_ENTRY, {0, 4} };
    sprite = Ov_ObjCreation::BuildRunningSprite(dataPlayer, m_Renderer.spriteTileMap, spriteGroup.get(), mapGroup.get(), runGroup.get(), &m_Renderer.spriteRenderer);
    spriteGroup->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, &m_Renderer.spriteRenderer);
    sprite->m_LastPermissionPtr = World::GetTilePermission(sprite->m_CurrentLevel, sprite->m_Tile, sprite->m_WorldLevel);

    std::shared_ptr<PlayerMove> walk(new PlayerMove(&sprite->m_CurrentLevel, &sprite->m_XPos, &sprite->m_ZPos, &sprite->m_Tile.x, &sprite->m_Tile.z));
    std::shared_ptr<PlayerCameraLock> spCam(new PlayerCameraLock(&sprite->m_XPos, &sprite->m_YPos, &sprite->m_ZPos, &m_Renderer.camera));
    std::shared_ptr<UpdateGlobalLevel> globLev(new UpdateGlobalLevel(&m_CurrentLevel, &sprite->m_CurrentLevel));
    walk->setInput(m_Input);
    walk->setSpriteData(sprite);
    m_LocationX = &sprite->m_Tile.x;
    m_LocationZ = &sprite->m_Tile.z;

    //Player is always first object pushed on overworld load - makes searching easier
    m_ObjManager.pushUpdateHeap(walk, &sprite->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(globLev, &sprite->m_UpdateComps);
    m_ObjManager.pushRenderHeap(spCam, &sprite->m_RenderComps);
    m_ObjManager.pushGameObject(sprite, "Player");

    //Init levels
    m_Levels.InitialiseGlobalObjects();
    m_Levels.LoadLevel(m_CurrentLevel);

    m_Renderer.generateAtlas();

    m_DataLoaded = true;
}

void Overworld::purgeRequiredData() {
    m_Renderer.purgeData();
    m_ObjManager.reset();
    //m_Fonts->clearFonts();
    m_Levels.UnloadAll();
    m_DataLoaded = false;
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

    //test
    m_CurrentLevelStr = "Current Level: " + std::to_string((int)m_CurrentLevel);
    m_CurrentTileStr = "Current Tile: " + std::to_string((int)*m_LocationX) + ", " + std::to_string((int)*m_LocationZ);
    m_ObjectCountStr = "Objects: " + std::to_string(m_ObjManager.getObjectCount());

    //End timer
    double timeTaken = EngineTimer::EndTimer(ts) * 1000.0;
    
    if (m_Sample)
    {
        m_UpdateTime = "Time to update: " + std::to_string(timeTaken) + "ms";
    }
}

void Overworld::render() {
    //ImGUI - I don't know why having this here increases speed
    //I will remove when I get to the bottom of it
    GameGUI::StartFrame();
    GameGUI::EndFrame();

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
    if (m_TextBuff.showTextBox)
    {
        m_TextBoxGUI.render();
    }
    GameGUI::ResetStyle();
    m_DebugGUI.render();

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
            if (m_Renderer.lightScene == 1)
            {
                m_Renderer.lightScene = 0;
            }
            else
            {
                m_Renderer.lightScene = 1;
            }
        }
    }
}

void Overworld::startBattle()
{
    this->setActive(false);
    m_StartBattle(true);
}