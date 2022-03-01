#include "game/states/Overworld.h"

void Overworld::init(int width, int height, World::LevelID levelEntry) {
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
    std::shared_ptr<SpriteRenderGroup> spriteGroup(new SpriteRenderGroup());
    std::shared_ptr<TilePositionGroup> tileGroup(new TilePositionGroup());

    std::shared_ptr<OverworldSprite> sprite(new OverworldSprite(0.0f, 0.0f, 0.0f, World::TILE_SIZE, World::TILE_SIZE));
    spriteGroup->addComponent(&sprite->m_RenderComps, &sprite->m_Sprite, &m_SpriteRenderer);
    
    //Sprite map test
    std::shared_ptr<SpriteMap> spriteMap(new SpriteMap(&sprite->m_Sprite, &sprite->m_AnimationOffsetX, &sprite->m_AnimationOffsetY, &m_SpriteTileMap));
    std::shared_ptr<UpdateSpriteFacing> updateFace(new UpdateSpriteFacing(&sprite->m_AnimationOffsetY, &sprite->m_Direction));
    std::shared_ptr<UpdateSpriteRunning> updateWalk(new UpdateSpriteRunning(&sprite->m_AnimationOffsetY, &sprite->m_AnimationOffsetX, &sprite->m_Direction, &sprite->m_Walking, &sprite->m_Running));

    std::shared_ptr<PlayerMove> walk(new PlayerMove(&sprite->m_CurrentLevel, &sprite->m_XPos, &sprite->m_ZPos, &sprite->m_TileX, &sprite->m_TileZ));
    std::shared_ptr<PlayerCameraLock> spCam(new PlayerCameraLock(&sprite->m_XPos, &sprite->m_YPos, &sprite->m_ZPos, &m_Camera));
    walk->setPersistentInput(&HELD_SHIFT, &HELD_W, &HELD_S, &HELD_A, &HELD_D);
    walk->setSingleInput(&PRESSED_W, &PRESSED_S, &PRESSED_A, &PRESSED_D);
    walk->setSpriteData(&sprite->m_Walking, &sprite->m_Running, &sprite->m_Direction, &sprite->m_Sprite);
    walk->attachToObject(&sprite->m_UpdateComps);
    m_ObjManager.pushUpdateHeap(std::static_pointer_cast<UpdateComponent>(walk));
    m_ObjManager.pushUpdateHeap(std::static_pointer_cast<UpdateComponent>(spriteMap));
    m_ObjManager.pushUpdateHeap(std::static_pointer_cast<UpdateComponent>(updateWalk));
    m_ObjManager.pushRenderHeap(std::static_pointer_cast<RenderComponent>(spCam));
    m_ObjManager.pushGameObject(std::static_pointer_cast<GameObject>(sprite));
    m_ObjManager.pushRenderGroup(spriteGroup);
    m_ObjManager.pushUpdateGroup(tileGroup);

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
}