#include "game/states/Overworld.h"

void Overworld::init(int width, int height, World::LevelID levelEntry, FontContainer* fonts, FlagArray* flags, GameInput* input) {
    //Width and height
    m_Width = width; m_Height = height;

    //Flags
    m_Flags = flags;

    //Input
    m_Input = input;

    m_Renderer.initialiseRenderer(width, height);

    //test level
    m_Levels.InitialiseLevels(&m_ObjManager, &m_Renderer, m_Flags, &m_TextBuff, m_Input);

    //gui shit test
    m_Fonts = fonts;
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 20);
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 70);

    std::shared_ptr<GameGUI::GameTextBox> tb(new GameGUI::GameTextBox(m_Width / 1.3f, 300.0f, 0.0f + (m_Width / 2 - m_Width / 2.6f), m_Height - 375.0f, m_TextBuff.t1, m_TextBuff.t2));
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
    std::shared_ptr<UpdateComponentGroup<UpdateAnimationRunning>> runGroup(new UpdateComponentGroup<UpdateAnimationRunning>());
    std::shared_ptr<UpdateComponentGroup<UpdateAnimationFacing>> faceGroup(new UpdateComponentGroup<UpdateAnimationFacing>());
    std::shared_ptr<UpdateComponentGroup<UpdateAnimationWalking>> walkGroup(new UpdateComponentGroup<UpdateAnimationWalking>());
    std::shared_ptr<UpdateComponentGroup<NPC_RandWalk>> randWGroup(new UpdateComponentGroup<NPC_RandWalk>());

    //Add component groups
    m_ObjManager.pushRenderGroup(spriteGroup, "SpriteRender");
    m_ObjManager.pushUpdateGroup(tileGroup, "TilePosition");
    m_ObjManager.pushUpdateGroup(mapGroup, "SpriteMap");
    m_ObjManager.pushUpdateGroup(runGroup, "RunMap");
    m_ObjManager.pushUpdateGroup(faceGroup, "UpdateFacing");
    m_ObjManager.pushUpdateGroup(walkGroup, "UpdateWalking");
    m_ObjManager.pushUpdateGroup(randWGroup, "RandWalk");

    //Load level data
    m_Levels.BuildFirstLevel(m_CurrentLevel);

    //Add player and link pos to lighting
    OvSpr_SpriteData dataPlayer = { {3, 0},  World::WorldHeight::F0, World::LevelID::LEVEL_ENTRY, {0, 4} };
    sprite = Ov_ObjCreation::BuildRunningSprite(dataPlayer, m_Renderer.spriteTileMap, spriteGroup.get(), mapGroup.get(), runGroup.get(), &m_Renderer.spriteRenderer);
    spriteGroup->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, &m_Renderer.spriteRenderer);
   
    std::shared_ptr<PlayerMove> walk(new PlayerMove(&sprite->m_CurrentLevel, &sprite->m_XPos, &sprite->m_ZPos, &sprite->m_Tile.x, &sprite->m_Tile.z));
    std::shared_ptr<PlayerCameraLock> spCam(new PlayerCameraLock(&sprite->m_XPos, &sprite->m_YPos, &sprite->m_ZPos, &m_Renderer.camera));
    std::shared_ptr<UpdateGlobalLevel> globLev(new UpdateGlobalLevel(&m_CurrentLevel, &sprite->m_CurrentLevel));
    walk->setInput(m_Input);
    walk->setSpriteData(sprite);
    m_LocationX = &sprite->m_Tile.x;
    m_LocationZ = &sprite->m_Tile.z;

    m_ObjManager.pushUpdateHeap(walk, &sprite->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(globLev, &sprite->m_UpdateComps);
    m_ObjManager.pushRenderHeap(spCam, &sprite->m_RenderComps);
    m_ObjManager.pushGameObject(sprite, "Player");

    //Test grass
    std::shared_ptr<TallGrass> grass(new TallGrass());
    grass->m_LevelID = m_CurrentLevel;
    std::shared_ptr<TallGrassRenderComponent> grassRen(new TallGrassRenderComponent(&m_Renderer.grassRenderer, m_Renderer.worldTileMap.uvTile(0, 3), &grass->m_Grass));
    std::shared_ptr<TallGrassAnimationComponent> grassAnim(new TallGrassAnimationComponent(&grass->m_GrassLoc, &grass->m_LevelID, &grass->m_ActiveStates));
    std::shared_ptr<SpriteAnim> spriteAnim1(new SpriteAnim(8, 3));
    std::shared_ptr<SpriteAnim> spriteAnim2(new SpriteAnim(8, 3));
    std::shared_ptr<SpriteAnim> spriteAnim3(new SpriteAnim(8, 3));
    std::shared_ptr<SpriteAnim> spriteAnim4(new SpriteAnim(8, 3));
    std::shared_ptr<SpriteAnim> spriteAnim5(new SpriteAnim(8, 3));
    std::shared_ptr<SpriteAnim> spriteAnim6(new SpriteAnim(8, 3));
    std::shared_ptr<SpriteAnim> spriteAnim7(new SpriteAnim(8, 3));
    std::shared_ptr<SpriteAnim> spriteAnim8(new SpriteAnim(8, 3));
    grassRen->reserveGrass(8);
    grassRen->addGrass({ 0,0 }, { 2,2 }, World::WorldHeight::F0, m_CurrentLevel, &grass->m_GrassLoc, &grass->m_ActiveStates);
    grassRen->addGrass({ 0,0 }, { 2,3 }, World::WorldHeight::F0, m_CurrentLevel, &grass->m_GrassLoc, &grass->m_ActiveStates);
    grassRen->addGrass({ 0,0 }, { 1,3 }, World::WorldHeight::F0, m_CurrentLevel, &grass->m_GrassLoc, &grass->m_ActiveStates);
    grassRen->addGrass({ 0,0 }, { 1,2 }, World::WorldHeight::F0, m_CurrentLevel, &grass->m_GrassLoc, &grass->m_ActiveStates);
    grassRen->addGrass({ 0,0 }, { 3,2 }, World::WorldHeight::F0, m_CurrentLevel, &grass->m_GrassLoc, &grass->m_ActiveStates);
    grassRen->addGrass({ 0,0 }, { 3,3 }, World::WorldHeight::F0, m_CurrentLevel, &grass->m_GrassLoc, &grass->m_ActiveStates);
    grassRen->addGrass({ 0,0 }, { 4,3 }, World::WorldHeight::F0, m_CurrentLevel, &grass->m_GrassLoc, &grass->m_ActiveStates);
    grassRen->addGrass({ 0,0 }, { 4,2 }, World::WorldHeight::F0, m_CurrentLevel, &grass->m_GrassLoc, &grass->m_ActiveStates);
    grassRen->generateIndices();
    spriteAnim1->linkSprite(&grass->m_Grass.quads[0], (bool*)&grass->m_ActiveStates[0]);
    spriteAnim2->linkSprite(&grass->m_Grass.quads[1], (bool*)&grass->m_ActiveStates[1]);
    spriteAnim3->linkSprite(&grass->m_Grass.quads[2], (bool*)&grass->m_ActiveStates[2]);
    spriteAnim4->linkSprite(&grass->m_Grass.quads[3], (bool*)&grass->m_ActiveStates[3]);
    spriteAnim5->linkSprite(&grass->m_Grass.quads[4], (bool*)&grass->m_ActiveStates[4]);
    spriteAnim6->linkSprite(&grass->m_Grass.quads[5], (bool*)&grass->m_ActiveStates[5]);
    spriteAnim7->linkSprite(&grass->m_Grass.quads[6], (bool*)&grass->m_ActiveStates[6]);
    spriteAnim8->linkSprite(&grass->m_Grass.quads[7], (bool*)&grass->m_ActiveStates[7]);
    TileUV uv1 = m_Renderer.worldTileMap.uvTile(0, 3);
    TileUV uv2 = m_Renderer.worldTileMap.uvTile(0, 4);
    TileUV uv3 = m_Renderer.worldTileMap.uvTile(0, 5);
    spriteAnim1->setFrame(0, uv1);
    spriteAnim1->setFrame(1, uv2);
    spriteAnim1->setFrame(2, uv3);
    spriteAnim2->setFrame(0, uv1);
    spriteAnim2->setFrame(1, uv2);
    spriteAnim2->setFrame(2, uv3);
    spriteAnim3->setFrame(0, uv1);
    spriteAnim3->setFrame(1, uv2);
    spriteAnim3->setFrame(2, uv3);
    spriteAnim4->setFrame(0, uv1);
    spriteAnim4->setFrame(1, uv2);
    spriteAnim4->setFrame(2, uv3);
    spriteAnim5->setFrame(0, uv1);
    spriteAnim5->setFrame(1, uv2);
    spriteAnim5->setFrame(2, uv3);
    spriteAnim6->setFrame(0, uv1);
    spriteAnim6->setFrame(1, uv2);
    spriteAnim6->setFrame(2, uv3);
    spriteAnim7->setFrame(0, uv1);
    spriteAnim7->setFrame(1, uv2);
    spriteAnim7->setFrame(2, uv3);
    spriteAnim8->setFrame(0, uv1);
    spriteAnim8->setFrame(1, uv2);
    spriteAnim8->setFrame(2, uv3);

    m_ObjManager.pushRenderHeap(grassRen, &grass->m_RenderComps);
    m_ObjManager.pushRenderHeap(grassAnim, &grass->m_RenderComps);
    m_ObjManager.pushUpdateHeap(spriteAnim1, &grass->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(spriteAnim2, &grass->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(spriteAnim3, &grass->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(spriteAnim4, &grass->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(spriteAnim5, &grass->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(spriteAnim6, &grass->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(spriteAnim7, &grass->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(spriteAnim8, &grass->m_UpdateComps);

    m_ObjManager.pushGameObject(grass, "Level0_Grass");

    //Test warp tile
    std::shared_ptr<WarpTile> warp(new WarpTile());
    std::shared_ptr<WarpTileUpdateComponent> warpUpdate(new WarpTileUpdateComponent(sprite.get(), { 10,2 }, World::WorldHeight::F0, {0,4}, World::WorldHeight::F1, World::LevelID::LEVEL_TEST));
    std::function<void(World::LevelID)> ld = std::bind(&World::LevelContainer::LoadLevel, &m_Levels, std::placeholders::_1);
    std::function<void(World::LevelID)> uld = std::bind(&World::LevelContainer::UnloadLevel, &m_Levels, std::placeholders::_1);
    warpUpdate->setLoadingFuncs(ld, uld);

    m_ObjManager.pushUpdateHeap(warpUpdate, &warp->m_UpdateComps);
    m_ObjManager.pushGameObject(warp);

    m_Levels.InitialiseGlobalObjects();
    m_Levels.LoadLevel(m_CurrentLevel);

    m_Renderer.generateAtlas();

    m_DataLoaded = true;
}

void Overworld::purgeRequiredData() {
    m_Renderer.purgeData();
    m_DataLoaded = false;
}

void Overworld::update(double deltaTime, double time) {
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
    m_UpdateTime = "Time to update: " + std::to_string(timeTaken) + "ms";
}

void Overworld::render() {

    //Start timer
    auto ts = EngineTimer::StartTimer();

    GameGUI::StartFrame();

    //Renders
    m_Levels.render();
    m_ObjManager.render();
    
    //Draw scene
    m_Renderer.bufferRenderData();
    m_Renderer.draw();

    //IMGUI Test
    if (m_TextBuff.showTextBox)
    {
        m_TextBoxGUI.render();
    }
    GameGUI::ResetStyle();
    m_DebugGUI.render();

    GameGUI::EndFrame();

    double timeTaken = EngineTimer::EndTimer(ts) * 1000.0;
    m_RenderTime = "Time to render: " + std::to_string(timeTaken) + "ms";
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