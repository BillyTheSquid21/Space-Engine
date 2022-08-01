#include "game/states/Battle.h"

void BattleScene::init(float width, float height)
{  
    m_Width = width; m_Height = height;

    //Do things relative to screen size
    m_SpriteWidth = width / 9.6f;

    buildPlatformA(); buildPlatformB();
    float platform2Depth = -m_Height / 2.1f;
    background = CreateTextureQuad(0, 0, m_Width, m_Height, 0, 0, 1, 1);
    Translate<TextureVertex>(&background[0], (float)m_Width / -2.1f, (float)m_Height / 4.0f, platform2Depth - 300, GetVerticesCount(Shape::QUAD));
}

void BattleScene::buildPlatformA()
{
    //Work out positioning
    platform1Data.depth = -m_Height / 4.2f;
    platform1Data.elevation = -m_Height / 27.5f;
    platform1Data.offset = -m_SpriteWidth + m_Width / 55.0f;
    
    //Create
    platform1 = CreateTextureQuad(0, 0, m_SpriteWidth, m_SpriteWidth, 0, 0, 1, 1);
    AxialRotate<TextureVertex>(&platform1[0], glm::vec3(m_SpriteWidth / 2.0f, 0, 0), -90, GetVerticesCount(Shape::QUAD), Axis::X);
    Translate<TextureVertex>(&platform1[0], platform1Data.offset, platform1Data.elevation, platform1Data.depth, GetVerticesCount(Shape::QUAD));
}

void BattleScene::buildPlatformB()
{
    //Work out data
    platform2Data.depth = -m_Height / 2.1f;
    platform2Data.elevation = -m_Height / 27.5f;
    platform2Data.offset = -m_SpriteWidth + m_Width / 6.0f;

    //Create
    platform2 = CreateTextureQuad(0, 0, m_SpriteWidth, m_SpriteWidth, 0, 0, 1, 1);
    AxialRotate<TextureVertex>(&platform2[0], glm::vec3(m_SpriteWidth / 2.0f, 0, 0), -90, GetVerticesCount(Shape::QUAD), Axis::X);
    Translate<TextureVertex>(&platform2[0], platform2Data.offset, platform2Data.elevation, platform2Data.depth, GetVerticesCount(Shape::QUAD));
}

void BattleScene::setPokemonA(std::string name)
{
    //Get sprite dim
    std::string path = "res/textures/pokemon/back/" + name +".png";
    GifData dim = GetImageData(path.c_str());

    //Work out pokemon positioning
    float pokemonAWidth = (dim.width / dim.frames) * (m_SpriteWidth / 126.0f);
    float pokemonAHeight = dim.height * (m_SpriteWidth / 126.0f);
    float pokemonAOffset = platform1Data.offset + 25.0f * (m_SpriteWidth / pokemonAWidth);
    float pokemonAElevation = platform1Data.elevation + pokemonAHeight;
    float pokemonADepth = platform1Data.depth + (m_SpriteWidth / 2.0f) + pokemonAHeight / 5.0f; //For taller pokemon shifts them back a bit

    pokemonA = CreateTextureQuad(0, 0, pokemonAWidth, pokemonAHeight, 0, 0, 1.0f / dim.frames, 1);
    AxialRotate<TextureVertex>(&pokemonA[0], { pokemonAWidth / 2.0f, 0, 0 }, -15.0f, GetVerticesCount(Shape::QUAD), Axis::X);
    Translate<TextureVertex>(&pokemonA[0], pokemonAOffset, pokemonAElevation, pokemonADepth, GetVerticesCount(Shape::QUAD));

    //Anim
    pokemonAAnim = SpriteAnim<TextureVertex, Tex_Quad>(15, dim.frames);
    for (float i = 0; i < dim.frames; i += 1.0f)
    {
        TileUV frame = { i / dim.frames, 0, 1.0f / (dim.frames * 1.01f), 1 };
        pokemonAAnim.setFrame(i, frame);
    }
    pokemonAAnim.linkSprite(&pokemonA, &active);
    pokemonAAnim.loop = true;
}

void BattleScene::setPokemonB(std::string name)
{
    //Get sprite dim
    std::string path = "res/textures/pokemon/front/" + name + ".png";
    GifData dim = GetImageData(path.c_str());

    //Work out pokemon positioning
    float pokemonBWidth = (dim.width / dim.frames) * (m_SpriteWidth / 126.0f);
    float pokemonBHeight = dim.height * (m_SpriteWidth / 126.0f);
    float pokemonBOffset = platform2Data.offset + 18.0f * (m_SpriteWidth / pokemonBWidth);
    float pokemonBElevation = platform2Data.elevation + pokemonBHeight;
    float pokemonBDepth = platform2Data.depth + (m_SpriteWidth / 2.0f) + pokemonBHeight / 5.0f; //For taller pokemon shifts them back a bit

    pokemonB = CreateTextureQuad(0, 0, pokemonBWidth, pokemonBHeight, 0, 0, 1.0f / (float)dim.frames, 1);
    AxialRotate<TextureVertex>(&pokemonB[0], { pokemonBWidth / 2.0f, 0, 0 }, -15.0f, GetVerticesCount(Shape::QUAD), Axis::X);
    Translate<TextureVertex>(&pokemonB[0], pokemonBOffset, pokemonBElevation, pokemonBDepth, GetVerticesCount(Shape::QUAD));

    pokemonBAnim = SpriteAnim<TextureVertex, Tex_Quad>(15, dim.frames);
    for (float i = 0; i < dim.frames; i += 1.0f)
    {
        TileUV frame = { i / dim.frames, 0, 1.0f / (dim.frames * 1.01f), 1 };
        pokemonBAnim.setFrame(i, frame);
    }
    pokemonBAnim.linkSprite(&pokemonB, &active);
    pokemonBAnim.loop = true;
}

void BattleScene::render(Render::Renderer<TextureVertex>& tex, Render::Renderer<TextureVertex>& back, Render::Renderer<TextureVertex>& pok1, Render::Renderer<TextureVertex>& pok2)
{
    tex.commit(&platform1[0], GetFloatCount<TextureVertex>(Shape::QUAD), &Primitive::Q_IND[0], Primitive::Q_IND_COUNT);
    tex.commit(&platform2[0], GetFloatCount<TextureVertex>(Shape::QUAD), &Primitive::Q_IND[0], Primitive::Q_IND_COUNT);
    back.commit(&background[0], GetFloatCount<TextureVertex>(Shape::QUAD), &Primitive::Q_IND[0], Primitive::Q_IND_COUNT);
    pok1.commit(&pokemonA[0], GetFloatCount<TextureVertex>(Shape::QUAD), &Primitive::Q_IND[0], Primitive::Q_IND_COUNT);
    pok2.commit(&pokemonB[0], GetFloatCount<TextureVertex>(Shape::QUAD), &Primitive::Q_IND[0], Primitive::Q_IND_COUNT);
}

void BattleScene::update(double deltaTime)
{
    pokemonAAnim.update(deltaTime);
    pokemonBAnim.update(deltaTime);
}

void Battle::init(int width, int height, PlayerData* data, FontContainer* fonts, FlagArray* flags, GameInput* input) {
    //Width and height
    m_Width = width; m_Height = height;

    //Get pool
    m_Pool = MtLib::ThreadPool::Fetch();

    //Flags
    m_Flags = flags;

    //Data
    m_Data = data;

    //Input
    m_Input = input; m_Battle->linkProgressButtons(&m_Input->PRESSED_E, &m_Input->PRESSED_X);

    //Renderer
    m_Renderer.initialiseRenderer(width, height);

    //Platform test
    m_Scene.init(width, height);

    //Fonts
    m_Fonts = fonts;
    m_Fonts->loadFont("res\\fonts\\PokemonXY\\PokemonXY.ttf", "boxfont", 45);

    //HUD
    std::shared_ptr<GameGUI::HUD> hud(new GameGUI::HUD(width, height, 0, 0));
    m_HUD.setBase(hud);
    std::shared_ptr<BattleGUI> battleGUI(new BattleGUI());
    battleGUI->setNest(0);
    battleGUI->m_Height = height / 3.2f;
    battleGUI->m_Width = width / 3.5f;
    battleGUI->m_XPos = width - battleGUI->m_Width - 10.0f;
    battleGUI->m_YPos = height - battleGUI->m_Height - 10.0f;
    battleGUI->setFontContainer(m_Fonts);
    m_HUD.addElement(battleGUI);
    m_GUI = battleGUI;
    m_GUI->setMoveTriggers(m_MoveTriggers);
    m_GUI->linkSwitchPkm(&m_ChangePkm);

    EngineLog("Battle scene loaded");
}

void Battle::startBattle(Party* partyA, Party* partyB)
{
    this->setActive(true);
    std::shared_ptr<PokemonBattle> battle(new PokemonBattle());
    m_Battle = battle;
    m_Battle->setParties(partyA, partyB);
    m_PlayerParty = partyA; m_EnemyParty = partyB;
    PokemonBattle::textBuffer.clear();
}

void Battle::setOverworldFunction(std::function<void(bool)> overworld)
{
    //Overworld
    m_OverworldEnable = overworld;
}

void Battle::loadRequiredData()
{
    m_Renderer.loadRendererData();
    m_Battle->linkHealthGui(std::bind(&BattleGUI::setHealthPerc, m_GUI, std::placeholders::_1, std::placeholders::_2));
    m_Exit = false;

    PokemonDataBank::loadData(PkmDataType::SPECIES_INFO);
    setPokemonA(m_PlayerParty->at(0).id);
    setPokemonB(m_EnemyParty->at(0).id);
    PokemonDataBank::unloadData(PkmDataType::SPECIES_INFO);

    m_DataLoaded = true;
}

void Battle::purgeRequiredData()
{
    m_Renderer.purgeData();
    m_DataLoaded = false;
}

void Battle::setPokemonA(uint16_t id)
{
    std::string name = PokemonDataBank::GetPokemonName(id);
    m_Scene.setPokemonA(name);
    m_Renderer.pokemonATexture.deleteTexture();
    m_Renderer.loadPokemonTextureA(name);

    //Gui
    m_GUI->setNameA(m_Battle->getNameA());
    m_GUI->setLevelA(m_Battle->getLevelA());
    m_GUI->setMoves(m_Battle->getMovesA());
    m_GUI->setHealthPercA(m_Battle->getHealthPercA());
}

void Battle::setPokemonB(uint16_t id)
{
    std::string name = PokemonDataBank::GetPokemonName(id);
    m_Scene.setPokemonB(name);
    m_Renderer.pokemonBTexture.deleteTexture();
    m_Renderer.loadPokemonTextureB(name);

    //Gui
    m_GUI->setNameB(m_Battle->getNameB());
    m_GUI->setLevelB(m_Battle->getLevelB());
    m_GUI->setHealthPercB(m_Battle->getHealthPercB());
}

//Temp
void Battle::update(double deltaTime, double time)
{
    //Check selected move - -1 if none
    for (int i = 0; i < (int)MoveSlot::SLOT_NULL; i++)
    {
        if (m_MoveTriggers[i])
        {
            selectedMove = (MoveSlot)i;
            break;
        }
    }
    //Check if pokemon switched
    if (m_ChangePkm)
    {
        for (int i = 0; i < m_PlayerParty->size(); i++)
        {
            if (m_PlayerParty->at(i).health <= 0 || m_PlayerParty->at(i).id == -1)
            {
                continue;
            }
            if (!m_Battle->checkFaintA(false))
            {
                selectedMove = MoveSlot::SLOT_CHANGE;
            }
            m_Battle->setActiveA(i);
            PokemonDataBank::loadData(PkmDataType::SPECIES_INFO);
            setPokemonA(m_PlayerParty->at(i).id);
            m_Pool->Run(PokemonDataBank::unloadData,PkmDataType::SPECIES_INFO);
            m_ChangePkm = false;
            break;
        }
    }
    if (selectedMove != MoveSlot::SLOT_NULL && (selectedMove == MoveSlot::SLOT_CHANGE || m_Battle->checkMoveValid(selectedMove)))
    {
        if (!m_Battle->isUpdating())
        {
            std::function<void(MoveSlot)> runFunc = std::bind(&PokemonBattle::run, m_Battle, std::placeholders::_1);
            m_Pool->Run(runFunc, selectedMove);
        }
    }
    selectedMove = MoveSlot::SLOT_NULL;

    m_Scene.update(deltaTime);

    if (m_Exit || m_Battle->isOver())
    {
        endBattle();
    }
}

void Battle::render()
{
    //Scene
    m_Scene.render(m_Renderer.worldRenderer, m_Renderer.backgroundRenderer, m_Renderer.pokemonARenderer, m_Renderer.pokemonBRenderer);

    m_Renderer.bufferRenderData();
    m_Renderer.draw();

    //IMGUI Test
    GameGUI::StartFrame();
    m_HUD.render();
    GameGUI::EndFrame();
}

void Battle::handleInput(int key, int scancode, int action, int mods)
{
    m_Input->handleInput(key, scancode, action, mods);
}

void Battle::endBattle()
{
    this->setActive(false);
    m_OverworldEnable(true);
}