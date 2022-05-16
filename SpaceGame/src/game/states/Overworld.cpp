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
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 70);

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

    //Add player
    OvSpr_SpriteData dataPlayer = { {3, 0},  World::WorldHeight::F0, World::LevelID::LEVEL_ENTRY, {0, 4} };
    sprite = Ov_ObjCreation::BuildRunningSprite(dataPlayer, m_Renderer.spriteTileMap, spriteGroup.get(), mapGroup.get(), runGroup.get(), &m_Renderer.spriteRenderer);
    spriteGroup->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, &m_Renderer.spriteRenderer);

    std::shared_ptr<PlayerMove> walk(new PlayerMove(&sprite->m_CurrentLevel, &sprite->m_XPos, &sprite->m_ZPos, &sprite->m_TileX, &sprite->m_TileZ));
    std::shared_ptr<PlayerCameraLock> spCam(new PlayerCameraLock(&sprite->m_XPos, &sprite->m_YPos, &sprite->m_ZPos, &m_Renderer.camera));
    walk->setInput(m_Input);
    walk->setSpriteData(sprite);

    m_ObjManager.pushUpdateHeap(walk, &sprite->m_UpdateComps);
    m_ObjManager.pushRenderHeap(spCam, &sprite->m_RenderComps);
    m_ObjManager.pushGameObject(sprite, "Player");

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
    m_Input->update(deltaTime);

    //update objects
    m_ObjManager.update(deltaTime);
}

void Overworld::render() {
    Render::Renderer<ColorTextureVertex>::clearScreen();

    GameGUI::StartFrame();

    //Bind shader program
    m_Renderer.bindShader();

    //Renders
    m_Levels.render();
    m_ObjManager.render();
    
    //Draw scene
    m_Renderer.bufferRenderData();
    m_Renderer.draw();

    //IMGUI Test
    GameGUI::TextBox gui(m_Width / 1.3f, 300.0f, 0.0f + (m_Width / 2 - m_Width / 2.6f), m_Height - 375.0f);
    gui.setFontContainer(m_Fonts);
    if (m_TextBuff.showTextBox)
    {
        gui.setStyle();
        gui.run(m_TextBuff.t1, m_TextBuff.t2);
    }
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
}