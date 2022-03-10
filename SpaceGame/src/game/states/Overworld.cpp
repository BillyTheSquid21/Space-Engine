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

    //Camera
    m_Camera.moveUp(World::TILE_SIZE * 5);
    m_Camera.panYDegrees(45.0f);

    //test level
    level.buildLevel(10, 10, &m_WorldRenderer, &m_OverworldTileMap);

    //test object system
    std::shared_ptr<RenderComponentGroup<SpriteRender>> spriteGroup(new RenderComponentGroup<SpriteRender>());
    std::shared_ptr<UpdateComponentGroup<TilePosition>> tileGroup(new UpdateComponentGroup<TilePosition>());

    OvSpr_SpriteData data = { {3, 0},  World::WorldLevel::F0, World::LevelID::LEVEL_ENTRY, {0, 4} };
    std::shared_ptr<OvSpr_RunningSprite> sprite = Ov_ObjCreation::BuildRunningSprite(data, m_SpriteTileMap);
    spriteGroup->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, &m_SpriteRenderer);
    
    //Sprite map test
    std::shared_ptr<SpriteMap> spriteMap(new SpriteMap(&sprite->m_Sprite, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &m_SpriteTileMap, { 0, 4 }));
    std::shared_ptr<UpdateAnimationRunning> updateWalk(new UpdateAnimationRunning(&sprite->m_AnimationOffsetY, &sprite->m_AnimationOffsetX, &sprite->m_Direction, &sprite->m_Walking, &sprite->m_Running));

    std::shared_ptr<PlayerMove> walk(new PlayerMove(&sprite->m_CurrentLevel, &sprite->m_XPos, &sprite->m_ZPos, &sprite->m_TileX, &sprite->m_TileZ));
    std::shared_ptr<PlayerCameraLock> spCam(new PlayerCameraLock(&sprite->m_XPos, &sprite->m_YPos, &sprite->m_ZPos, &m_Camera));
    walk->setPersistentInput(&HELD_SHIFT, &HELD_W, &HELD_S, &HELD_A, &HELD_D);
    walk->setSingleInput(&PRESSED_W, &PRESSED_S, &PRESSED_A, &PRESSED_D);
    walk->setSpriteData(&sprite->m_Walking, &sprite->m_Running, &sprite->m_Direction, &sprite->m_Sprite);
    
    //Setup test npc
    OvSpr_SpriteData data2 = { {3, 3},  World::WorldLevel::F0, World::LevelID::LEVEL_ENTRY, {0, 0} };
    std::shared_ptr<OvSpr_WalkingSprite> npc(new OvSpr_WalkingSprite(data2));
    spriteGroup->addComponent(&npc->m_RenderComps, &npc->m_Sprite, &m_SpriteRenderer);
    std::shared_ptr<UpdateAnimationWalking> updateFace(new UpdateAnimationWalking(&npc->m_AnimationOffsetY, &npc->m_AnimationOffsetX, &npc->m_Direction, &npc->m_Walking));
    std::shared_ptr<NPC_RandWalk> randWalk(new NPC_RandWalk(&npc->m_Direction, &npc->m_Busy, &npc->m_Walking, &npc->m_Sprite));
    randWalk->setLocation(&npc->m_TileX, &npc->m_TileZ, &npc->m_XPos, &npc->m_ZPos, &npc->m_CurrentLevel);
    std::shared_ptr<SpriteMap> npcMap(new SpriteMap(&npc->m_Sprite, &npc->m_AnimationOffsetX, &npc->m_AnimationOffsetY, &m_SpriteTileMap, { 0,0 }));

    //Test script
    Script script(new ScriptElement[2]);
    script[0].instruction = ScriptInstruction::OPEN_MSG_BOX;
    script[1].instruction = ScriptInstruction::MSG;
    std::shared_ptr<NPC_Script> npcscp(new NPC_Script(script, 2));
    npcscp->setupText(&m_TextBuff.t1, &m_TextBuff.t2, &FindMessage, &m_GUIEnabled.showTextBox);
   
    
    m_ObjManager.pushUpdateHeap(walk, &sprite->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(spriteMap, &sprite->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(npcMap, &npc->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(updateWalk, &sprite->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(updateFace, &npc->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(randWalk, &npc->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(npcscp, &npc->m_UpdateComps);
    m_ObjManager.pushRenderHeap(spCam, &sprite->m_RenderComps);
    m_ObjManager.pushGameObject(sprite);
    m_ObjManager.pushGameObject(npc);
    m_ObjManager.pushRenderGroup(spriteGroup, "SpriteRender");
    m_ObjManager.pushUpdateGroup(tileGroup, "TilePosition");

    //gui shit test
    m_Fonts = fonts;
    m_Fonts->loadFont("res\\fonts\\Newsgeek\\Newsgeek.ttf", "default", 70);
    m_Fonts->loadFont("res\\fonts\\Newsgeek\\Newsgeek.ttf", "default", 35);

    EngineLog("Overworld loaded: ", (int)m_CurrentLevel);
}

void Overworld::loadRequiredData() {
    //Shader
    m_Shader.create("res/shaders/DefaultTexture.glsl");

    //Set texture uniform
    m_Shader.setUniform("u_Texture", 1);

    //Plane
    m_Plane.setRenderer(&m_WorldRenderer);
    m_Plane.generatePlaneXZ(0.0f, 0.0f, World::TILE_SIZE * 32, World::TILE_SIZE * 32, World::TILE_SIZE);
    UVData data = m_OverworldTileMap.uvTile(0, 0);
    m_Plane.texturePlane(data.uvX, data.uvY, data.uvWidth, data.uvHeight);
    
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



    m_DataLoaded = true;
}

void Overworld::purgeRequiredData() {
    m_PlaneTexture.unbind();
    m_Plane.purgeData();
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
    Renderer<ColorTextureVertex>::clearScreen();

    GameGUI::StartFrame();

    //Bind shader program
    m_Shader.bind();

    //Renders
    level.render();
    m_ObjManager.render();
    
    m_Camera.sendCameraUniforms(m_Shader);

    m_Shader.setUniform("u_Texture", 0);
    m_WorldRenderer.drawPrimitives(m_Shader);
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