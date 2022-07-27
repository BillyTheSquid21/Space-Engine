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

    EngineLog("Battle scene loaded");
}

void Battle::createGUI()
{
    //Gui test
    gui = GameGUI::GUIContainer();

    levelA = std::to_string(m_PlayerParty->at(0).level);
    levelB = std::to_string(m_EnemyParty->at(0).level);

    std::shared_ptr<GameGUI::HUD> hud(new GameGUI::HUD(m_Width, m_Height, 0, 0));
    gui.setBase(hud);

    const float hudW = 400.0f; const float hudH = 200.0f;
    const float buffer = 40.0f;

    //Add health box 1:
    std::shared_ptr<GameGUI::Divider> enemy(new GameGUI::Divider());
    enemy->m_Width = hudW; enemy->m_Height = hudH;
    enemy->setNest(0);
    enemy->m_XPos = buffer; enemy->m_YPos = buffer;
    gui.addElement(enemy);
    std::shared_ptr<GameGUI::TextBox> enLev(new GameGUI::TextBox(levelB));
    enLev->setNest(0);
    gui.addElement(enLev);
    std::shared_ptr<GameGUI::TextBox> enName(new GameGUI::TextBox(nameB));
    enName->setNest(0);
    gui.addElement(enName);
    std::shared_ptr<GameGUI::TextBox> enHealth(new GameGUI::TextBox(healthB));
    enHealth->setNest(0);
    gui.addElement(enHealth);
    std::shared_ptr<GameGUI::TextBox> enCond(new GameGUI::TextBox(conditionB));
    enCond->setNest(0);
    gui.addElement(enCond);

    //Add health box 2:
    std::shared_ptr<GameGUI::Divider> player(new GameGUI::Divider());
    player->m_Width = hudW; player->m_Height = hudH;
    player->setNest(1);
    player->m_XPos = m_Width - hudW - buffer;
    player->m_YPos = m_Height - hudH - buffer;
    gui.addElement(player);
    std::shared_ptr<GameGUI::TextBox> plLev(new GameGUI::TextBox(levelA));
    plLev->setNest(1);
    gui.addElement(plLev);
    std::shared_ptr<GameGUI::TextBox> plName(new GameGUI::TextBox(nameA));
    plName->setNest(1);
    gui.addElement(plName);
    std::shared_ptr<GameGUI::TextBox> plHealth(new GameGUI::TextBox(healthA));
    plHealth->setNest(1);
    gui.addElement(plHealth);
    std::shared_ptr<GameGUI::TextBox> plCond(new GameGUI::TextBox(conditionA));
    plCond->setNest(1);
    gui.addElement(plCond);

    //Move buttons
    std::string move1 = (*m_PlayerParty)[0].moves[0].identifier;
    std::string move2 = (*m_PlayerParty)[0].moves[1].identifier;
    std::string move3 = (*m_PlayerParty)[0].moves[2].identifier;
    std::string move4 = (*m_PlayerParty)[0].moves[3].identifier;
    std::shared_ptr<GameGUI::Button> but1(new GameGUI::Button(move1, &moveTriggers[0]));
    but1->setNest(1);
    but1->m_XPos = 8.0f; but1->m_YPos = 115.0f;
    gui.addElement(but1);
    std::shared_ptr<GameGUI::Button> but2(new GameGUI::Button(move2, &moveTriggers[1]));
    but2->setNest(1);
    but2->m_XPos = 100.0f; but2->m_YPos = 115.0f;
    gui.addElement(but2);
    std::shared_ptr<GameGUI::Button> but3(new GameGUI::Button(move3, &moveTriggers[2]));
    but3->setNest(1);
    but3->m_XPos = 8.0f; but3->m_YPos = 150.0f;
    gui.addElement(but3);
    std::shared_ptr<GameGUI::Button> but4(new GameGUI::Button(move4, &moveTriggers[3]));
    but4->setNest(1);
    but4->m_XPos = 100.0f; but4->m_YPos = 150.0f;
    gui.addElement(but4);
    std::shared_ptr<GameGUI::Button> but5(new GameGUI::Button("Exit", &m_Exit));
    but5->setNest(1);
    but5->m_XPos = 8.0f;
    but5->m_YPos = 190.0f;
    gui.addElement(but5);

    //Add text box
    std::shared_ptr<GameGUI::Divider> textBoxBase(new GameGUI::Divider());
    textBoxBase->m_Width = m_Width * 0.3f; textBoxBase->m_Height = hudH;
    textBoxBase->setNest(2);
    textBoxBase->m_XPos = but4->m_XPos + textBoxBase->m_Width + m_Width * 0.1f;
    textBoxBase->m_YPos = m_Height - hudH - buffer;
    gui.addElement(textBoxBase);
    std::shared_ptr<GameGUI::TextBox> textBox1(new GameGUI::TextBox(BattleTextBuffer::m_Line1));
    textBox1->setNest(2);
    gui.addElement(textBox1);
    std::shared_ptr<GameGUI::TextBox> textBox2(new GameGUI::TextBox(BattleTextBuffer::m_Line2));
    textBox2->setNest(2);
    gui.addElement(textBox2);
}

void Battle::startBattle(Party* partyA, Party* partyB)
{
    this->setActive(true);
    std::shared_ptr<PokemonBattle> battle(new PokemonBattle());
    m_Battle = battle;
    m_Battle->setParties(partyA, partyB);
    m_PlayerParty = partyA; m_EnemyParty = partyB;
    BattleTextBuffer::clearText();
}

void Battle::setOverworldFunction(std::function<void(bool)> overworld)
{
    //Overworld
    m_OverworldEnable = overworld;
}

void Battle::loadRequiredData()
{
    m_Renderer.loadRendererData();
    createGUI();
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
}

void Battle::setPokemonB(uint16_t id)
{
    std::string name = PokemonDataBank::GetPokemonName(id);
    m_Scene.setPokemonB(name);
    m_Renderer.pokemonBTexture.deleteTexture();
    m_Renderer.loadPokemonTextureB(name);
}

//Temp
void Battle::update(double deltaTime, double time)
{
    //Check selected move - -1 if none
    bool progress = false;
    for (int i = 0; i < (int)MoveSlot::SLOT_NULL; i++)
    {
        if (moveTriggers[i])
        {
            selectedMove = (MoveSlot)i;
            progress = true;
            break;
        }
    }
    if (selectedMove != MoveSlot::SLOT_NULL && m_Battle->checkMoveValid(selectedMove))
    {
        if (!m_Battle->isUpdating())
        {
            std::function<void(MoveSlot)> runFunc = std::bind(&PokemonBattle::run, m_Battle, std::placeholders::_1);
            m_Pool->Run(runFunc, selectedMove);
        }
    }
    selectedMove = MoveSlot::SLOT_NULL;

    m_Scene.update(deltaTime);

    //Update health
    healthA = health + std::to_string(m_Battle->getHealthA());
    healthB = health + std::to_string(m_Battle->getHealthB());

    //Update status
    conditionA = status + std::to_string(m_Battle->getStatusA());
    conditionB = status + std::to_string(m_Battle->getStatusB());

    //Update nickname
    nameA = m_Battle->getNameA();
    nameB = m_Battle->getNameB();

    levelA = "Lv 50";
    levelB = "Lv 50";

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
    GameGUI::ResetStyle();
    gui.render();
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