#include "game/states/Overworld.h"

void Overworld::init(int width, int height, World::LevelID levelEntry, FontContainer* fonts) {
    //Width and height
    m_Width = width; m_Height = height;

    //Renderer setup
    m_Camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

    //World Renderer
    m_WorldRenderer.setLayout<float>(3, 2);
    m_WorldRenderer.setDrawingMode(GL_TRIANGLES);
    m_WorldRenderer.generate((float)width, (float)height, &m_Camera);

    //Sprite Renderer
    m_SpriteRenderer.setLayout<float>(3, 2);
    m_SpriteRenderer.setDrawingMode(GL_TRIANGLES);
    m_SpriteRenderer.generate((float)width, (float)height, &m_Camera);

    //Model Renderer
    m_ModelRenderer.setLayout<float>(3, 2);
    m_ModelRenderer.setDrawingMode(GL_TRIANGLES);
    m_ModelRenderer.generate((float)width, (float)height, &m_Camera);

    //Camera
    m_Camera.moveUp(World::TILE_SIZE * 5);
    m_Camera.panYDegrees(45.0f);

    //test level
    m_Levels.InitialiseLevels();
    m_Levels.LoadLevel(m_CurrentLevel, &m_WorldRenderer, &m_OverworldTileMap);

    //gui shit test
    m_Fonts = fonts;
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 70);

    EngineLog("Overworld loaded: ", (int)m_CurrentLevel);
}

void Overworld::loadRequiredData() {
    //Shader
    m_Shader.create("res/shaders/DefaultTexture.glsl");

    //Set texture uniform
    m_Shader.setUniform("u_Texture", 1);

    //Load world texture
    m_PlaneTexture.loadTexture("res/textures/OW.png");
    m_PlaneTexture.generateTexture(0);
    m_PlaneTexture.bind();
    m_PlaneTexture.clearBuffer();

    //Load sprite textures
    m_OWSprites.loadTexture("res/textures/Sprite.png");
    m_OWSprites.generateTexture(1);
    m_OWSprites.bind();
    m_OWSprites.clearBuffer();

    //Create groups
    //Add component groups
    std::shared_ptr<RenderComponentGroup<SpriteRender>> spriteGroup(new RenderComponentGroup<SpriteRender>());
    std::shared_ptr<UpdateComponentGroup<TilePosition>> tileGroup(new UpdateComponentGroup<TilePosition>());
    std::shared_ptr<UpdateComponentGroup<SpriteMap>> mapGroup(new UpdateComponentGroup<SpriteMap>());
    std::shared_ptr<UpdateComponentGroup<UpdateAnimationRunning>> runGroup(new UpdateComponentGroup<UpdateAnimationRunning>());

    //Add player
    OvSpr_SpriteData dataPlayer = { {3, 0},  World::WorldLevel::F0, World::LevelID::LEVEL_ENTRY, {0, 4} };
    std::shared_ptr<OvSpr_RunningSprite> sprite = Ov_ObjCreation::BuildRunningSprite(dataPlayer, m_SpriteTileMap, spriteGroup.get(), mapGroup.get(), runGroup.get(), &m_SpriteRenderer);
    spriteGroup->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, &m_SpriteRenderer);

    std::shared_ptr<PlayerMove> walk(new PlayerMove(&sprite->m_CurrentLevel, &sprite->m_XPos, &sprite->m_ZPos, &sprite->m_TileX, &sprite->m_TileZ));
    std::shared_ptr<PlayerCameraLock> spCam(new PlayerCameraLock(&sprite->m_XPos, &sprite->m_YPos, &sprite->m_ZPos, &m_Camera));
    walk->setPersistentInput(&HELD_SHIFT, &HELD_W, &HELD_S, &HELD_A, &HELD_D);
    walk->setSingleInput(&PRESSED_W, &PRESSED_S, &PRESSED_A, &PRESSED_D);
    walk->setSpriteData(&sprite->m_Walking, &sprite->m_Running, &sprite->m_Direction, &sprite->m_Sprite);

    m_ObjManager.pushUpdateHeap(walk, &sprite->m_UpdateComps);
    m_ObjManager.pushRenderHeap(spCam, &sprite->m_RenderComps);
    m_ObjManager.pushGameObject(sprite);

    std::shared_ptr<LoadingZone> lz(new LoadingZone());
    std::shared_ptr<LoadingZoneComponent> load(new LoadingZoneComponent(sprite.get(), World::LevelID::LEVEL_ENTRY, World::LevelID::LEVEL_TEST, &m_WorldRenderer, &m_OverworldTileMap));
    load->setLoadingFuncs(std::bind(&World::LevelContainer::LoadLevel, &m_Levels, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), std::bind(&World::LevelContainer::UnloadLevel, &m_Levels, std::placeholders::_1));

    m_ObjManager.pushRenderHeap(load, &lz->m_RenderComps);
    m_ObjManager.pushGameObject(lz);

    //Add component groups
    m_ObjManager.pushRenderGroup(spriteGroup, "SpriteRender");
    m_ObjManager.pushUpdateGroup(tileGroup, "TilePosition");
    m_ObjManager.pushUpdateGroup(mapGroup, "SpriteMap");
    m_ObjManager.pushUpdateGroup(runGroup, "RunMap");

    atlas.loadTexture("res/textures/OW.png", "OW");
    atlas.loadTexture("res/textures/willow.png", "SPRITE");
    atlas.loadTexture("res/textures/willow2.png", "SPRITE2");

    atlas.generateAtlas();
    atlas.generateTexture(2);
    atlas.bind();

    atlas.mapModelVerts(mod.getVertices(), mod.getVertCount(), "SPRITE");

    //model test
    mod.setRen(&m_ModelRenderer);
    modMat = glm::mat4(1.0f);
    modMat = glm::scale(modMat, {8.0f, 8.0f, 8.0f});

    m_DataLoaded = true;
}

void Overworld::purgeRequiredData() {
    m_PlaneTexture.unbind();
    m_DataLoaded = false;
}

void Overworld::update(double deltaTime, double time) {
    //Keeps stored to allow input to use
    if (m_HoldTimerActive)
    {
        m_TimeHeld += deltaTime;
    }

    //update objects
    m_ObjManager.update(deltaTime);
}

void Overworld::render() {
    Render::Renderer<ColorTextureVertex>::clearScreen();

    GameGUI::StartFrame();

    //Bind shader program
    m_Shader.bind();

    //Renders
    m_Levels.render();
    m_ObjManager.render();

    modMat = glm::rotate(modMat, glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_ModelRenderer.commitModelMat(modMat);
    m_ModelRenderer.commitModelMat(glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, -100.0f)));
    mod.render();
    
    m_Camera.sendCameraUniforms(m_Shader);

    m_Shader.setUniform("u_Texture", 0);
    m_WorldRenderer.drawPrimitives(m_Shader);
    m_Shader.setUniform("u_Texture", 2);
    m_ModelRenderer.drawPrimitives(m_Shader);
    m_Shader.setUniform("u_Texture", 1);
    m_SpriteRenderer.drawPrimitives(m_Shader);


    //IMGUI Test
    GameGUI::TextBox gui(m_Width / 1.3f, 300.0f, 0.0f + (m_Width / 2 - m_Width / 2.6f), m_Height - 375.0f);
    gui.setFontContainer(m_Fonts);
    if (m_GUIEnabled.showTextBox)
    {
        gui.setStyle();
        gui.run(m_TextBuff.t1, m_TextBuff.t2);
    }
    GameGUI::EndFrame();
}

void Overworld::handleInput(int key, int scancode, int action, int mods) {
    //Reset pressed
    PRESSED_A = false; PRESSED_D = false; PRESSED_S = false; PRESSED_W = false;

    if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS) {
            PRESSED_A = true;
            m_HoldTimerActive = true;
            if (HELD_SHIFT)
            {
                m_TimeHeld = s_TimeToHold;
            }
        }
        else if (action == GLFW_RELEASE) {
            HELD_A = false;
            m_TimeHeld = 0.0;
        }
        if (m_TimeHeld >= s_TimeToHold)
        {
            HELD_A = true;
            m_HoldTimerActive = false;
        }
    }
    if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS) {
            PRESSED_D = true;
            m_HoldTimerActive = true;
            if (HELD_SHIFT)
            {
                m_TimeHeld = s_TimeToHold;
            }
        }
        else if (action == GLFW_RELEASE) {
            HELD_D = false;
            m_TimeHeld = 0.0;
        }
        if (m_TimeHeld >= s_TimeToHold)
        {
            HELD_D = true;
            m_HoldTimerActive = false;
        }
    }
    if (key == GLFW_KEY_LEFT_CONTROL) {
        if (action == GLFW_PRESS) {
            HELD_CTRL = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_CTRL = false;
        }
    }
    if (key == GLFW_KEY_LEFT_SHIFT) {
        if (action == GLFW_PRESS) {
            HELD_SHIFT = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_SHIFT = false;
        }
    }
    if (key == GLFW_KEY_Q) {
        if (action == GLFW_PRESS) {
            HELD_Q = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_Q = false;
        }
    }
    if (key == GLFW_KEY_E) {
        if (action == GLFW_PRESS) {
            HELD_E = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_E = false;
        }
    }
    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS) {
            PRESSED_W = true;
            m_HoldTimerActive = true;
            if (HELD_SHIFT)
            {
                m_TimeHeld = s_TimeToHold;
            }
        }
        else if (action == GLFW_RELEASE) {
            HELD_W = false;
            m_TimeHeld = 0.0;
        }
        if (m_TimeHeld >= s_TimeToHold)
        {
            HELD_W = true;
            m_HoldTimerActive = false;
        }
    }
    if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS) {
            PRESSED_S = true;
            m_HoldTimerActive = true;
            if (HELD_SHIFT)
            {
                m_TimeHeld = s_TimeToHold;
            }
        }
        else if (action == GLFW_RELEASE) {
            HELD_S = false;
            m_TimeHeld = 0.0;
        }
        if (m_TimeHeld >= s_TimeToHold)
        {
            HELD_S = true;
            m_HoldTimerActive = false;
        }
    }

    //Debug
    if (key == GLFW_KEY_Y)
    {
        if (action == GLFW_PRESS)
        {
            m_WorldRenderer.setDrawingMode(GL_LINES);
        }
    }
    if (key == GLFW_KEY_U)
    {
        if (action == GLFW_PRESS)
        {
            m_WorldRenderer.setDrawingMode(GL_TRIANGLES);
        }
    }
}