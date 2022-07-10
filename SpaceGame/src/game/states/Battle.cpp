#include "game/states/Battle.h"

void BattleScene::init(float width, float height)
{  
    //Do things relative to screen size
    float spriteWidth = width / 9.6f;

    float platform1Depth = -height / 4.2f;
    float platform1Elevation = -height / 27.5f;
    float platform1Offset = -spriteWidth + width/55.0f;

    float platform2Depth = -height / 2.1f;
    float platform2Elevation = -height / 27.5f;
    float platform2Offset = -spriteWidth + width / 6.0f;

    platform1 = CreateTextureQuad(0, 0, spriteWidth, spriteWidth, 0, 0, 1, 1);
    AxialRotate<TextureVertex>(&platform1[0], glm::vec3(spriteWidth/2.0f, 0, 0), -90, GetVerticesCount(Shape::QUAD), Axis::X);
    Translate<TextureVertex>(&platform1[0], platform1Offset, platform1Elevation, platform1Depth, GetVerticesCount(Shape::QUAD));

    platform2 = CreateTextureQuad(0, 0, spriteWidth, spriteWidth, 0, 0, 1, 1);
    AxialRotate<TextureVertex>(&platform2[0], glm::vec3(spriteWidth / 2.0f, 0, 0), -90, GetVerticesCount(Shape::QUAD), Axis::X);
    Translate<TextureVertex>(&platform2[0], platform2Offset, platform2Elevation, platform2Depth, GetVerticesCount(Shape::QUAD));

    background = CreateTextureQuad(0, 0, width, height, 0, 0, 1, 1);
    Translate<TextureVertex>(&background[0], (float)width/-2.1f, (float)height/4.0f, platform2Depth-300, GetVerticesCount(Shape::QUAD));

    //Get sprite dim
    ImageDim dim = GetImageDimension("res/textures/pokemon/Torterra.png");

    //Work out pokemon positioning
    float pokemonAWidth = (dim.width / FRAME_COUNT) * (spriteWidth / 126.0f);
    float pokemonAHeight = dim.height * (spriteWidth / 126.0f);
    float pokemonAOffset = platform1Offset + 25.0f*(spriteWidth/pokemonAWidth);
    float pokemonAElevation = platform1Elevation + pokemonAHeight;
    float pokemonADepth = platform1Depth + (spriteWidth/2.0f) + pokemonAHeight/5.0f; //For taller pokemon shifts them back a bit

    pokemonA = CreateTextureQuad(0, 0, pokemonAWidth,  pokemonAHeight, 0, 0, 1.0f / 51.0f, 1);
    AxialRotate<TextureVertex>(&pokemonA[0], { pokemonAWidth / 2.0f, 0, 0 }, -15.0f, GetVerticesCount(Shape::QUAD), Axis::X);
    Translate<TextureVertex>(&pokemonA[0], pokemonAOffset, pokemonAElevation, pokemonADepth, GetVerticesCount(Shape::QUAD));

    dim = GetImageDimension("res/textures/pokemon/Chandelure2.png");

    //Work out pokemon positioning
    float pokemonBWidth = (dim.width / FRAME_COUNT) * (spriteWidth / 126.0f);
    float pokemonBHeight = dim.height * (spriteWidth / 126.0f);
    float pokemonBOffset = platform2Offset + 18.0f * (spriteWidth / pokemonBWidth);
    float pokemonBElevation = platform2Elevation + pokemonBHeight;
    float pokemonBDepth = platform2Depth + (spriteWidth / 2.0f) + pokemonBHeight / 5.0f; //For taller pokemon shifts them back a bit

    pokemonB = CreateTextureQuad(0, 0, pokemonBWidth, pokemonBHeight, 0, 0, 1.0f / 51.0f, 1);
    AxialRotate<TextureVertex>(&pokemonB[0], { pokemonAWidth / 2.0f, 0, 0 }, -15.0f, GetVerticesCount(Shape::QUAD), Axis::X);
    Translate<TextureVertex>(&pokemonB[0], pokemonBOffset, pokemonBElevation, pokemonBDepth, GetVerticesCount(Shape::QUAD));

    //Anim
    pokemonAAnim = SpriteAnim<TextureVertex, Tex_Quad>(12, 51);
    for (float i = 0; i < FRAME_COUNT; i+=1.0f)
    {
        TileUV frame = { i / FRAME_COUNT, 0, 1.0f / (FRAME_COUNT*1.01f), 1 };
        pokemonAAnim.setFrame(i, frame);
    }
    pokemonAAnim.linkSprite(&pokemonA, &active);
    pokemonAAnim.loop = true;

    pokemonBAnim = SpriteAnim<TextureVertex, Tex_Quad>(12, 51);
    for (float i = 0; i < FRAME_COUNT; i += 1.0f)
    {
        TileUV frame = { i / FRAME_COUNT, 0, 1.0f / (FRAME_COUNT * 1.01f), 1 };
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

//HUD
void HUD::openNest()
{
    ImGui::Begin("Debug Menu", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts[0]);
}

void HUD::closeNest()
{
    ImGui::PopFont();
}

void Battle::init(int width, int height, FontContainer* fonts, FlagArray* flags, GameInput* input) {
    //Width and height
    m_Width = width; m_Height = height;

    //Get pool
    m_Pool = MtLib::ThreadPool::Fetch();

    //Flags
    m_Flags = flags;

    //Input
    m_Input = input; m_Battle.linkProgressButtons(&m_Input->PRESSED_E, &m_Input->PRESSED_X);

    //Renderer
    m_Renderer.initialiseRenderer(width, height);

    //Platform test
    m_Scene.init(width, height);

    //Pkm test
    MtLib::ThreadPool* pool = MtLib::ThreadPool::Fetch();

    PokemonDataBank::loadData(PkmDataType::SPECIES_INFO);
    PokemonDataBank::loadData(PkmDataType::BASE_STATS);
    PokemonDataBank::loadData(PkmDataType::MOVE_INFO);
    PokemonDataBank::loadData(PkmDataType::POKEMON_TYPES);

    m_PlayerParty[0].id = 389;
    m_PlayerParty[0].moves[2].id = 89;
    m_PlayerParty[0].moves[1].id = 75;
    m_PlayerParty[0].moves[0].id = 89;
    m_PlayerParty[0].moves[3].id = 402;
    GeneratePokemon(m_PlayerParty[0].id, m_PlayerParty[0]);
    SetPkmStatsFromLevel(m_PlayerParty[0]);
    m_EnemyParty[0].id = 466;
    m_EnemyParty[0].moves[0].id = 7;
    m_EnemyParty[0].moves[0].status = StatusCondition::Burn;
    m_EnemyParty[0].moves[0].statusAcc = 50;
    GeneratePokemon(m_EnemyParty[0].id, m_EnemyParty[0]);
    SetPkmStatsFromLevel(m_EnemyParty[0]);
    m_Battle.setParties(m_PlayerParty, m_EnemyParty);

    pool->Run(PokemonDataBank::unloadData, PkmDataType::SPECIES_INFO);
    pool->Run(PokemonDataBank::unloadData, PkmDataType::BASE_STATS);
    pool->Run(PokemonDataBank::unloadData, PkmDataType::MOVE_INFO);
    pool->Run(PokemonDataBank::unloadData, PkmDataType::POKEMON_TYPES);

    //Gui test
    levelA = std::to_string(m_PlayerParty[0].level);
    levelB = std::to_string(m_EnemyParty[0].level);

    std::shared_ptr<HUD> hud(new HUD(m_Width, m_Height,0,0));
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
    std::string move1 = m_PlayerParty[0].moves[0].identifier;
    std::string move2 = m_PlayerParty[0].moves[1].identifier;
    std::string move3 = m_PlayerParty[0].moves[2].identifier;
    std::string move4 = m_PlayerParty[0].moves[3].identifier;
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

    //Add text box
    std::shared_ptr<GameGUI::Divider> textBoxBase(new GameGUI::Divider());
    textBoxBase->m_Width = m_Width*0.3f; textBoxBase->m_Height = hudH;
    textBoxBase->setNest(2);
    textBoxBase->m_XPos = but4->m_XPos + textBoxBase->m_Width + m_Width*0.1f;
    textBoxBase->m_YPos = m_Height - hudH - buffer;
    gui.addElement(textBoxBase);
    std::shared_ptr<GameGUI::TextBox> textBox1(new GameGUI::TextBox(BattleTextBuffer::m_Line1));
    textBox1->setNest(2);
    gui.addElement(textBox1);
    std::shared_ptr<GameGUI::TextBox> textBox2(new GameGUI::TextBox(BattleTextBuffer::m_Line2));
    textBox2->setNest(2);
    gui.addElement(textBox2);

    EngineLog("Battle scene loaded");
}

void Battle::loadRequiredData()
{
    m_Renderer.loadRendererData();

    m_DataLoaded = true;
}

void Battle::purgeRequiredData()
{
    m_Renderer.purgeData();
}

//Temp
bool first = true;
void Battle::update(double deltaTime, double time)
{
    //Check selected move - -1 if none
    bool progress = false;
    if (first)
    {
        BattleTextBuffer::pushText("A wild " + m_EnemyParty[0].nickname + " appeared!");
    }
    for (int i = 0; i < (int)MoveSlot::SLOT_NULL; i++)
    {
        if (moveTriggers[i])
        {
            selectedMove = (MoveSlot)i;
            progress = true;
            break;
        }
    }
    if (selectedMove != MoveSlot::SLOT_NULL && m_Battle.checkMoveValid(selectedMove))
    {
        if (!m_Battle.isUpdating())
        {
            std::function<void(MoveSlot)> runFunc = std::bind(&PokemonBattle::run, &m_Battle, std::placeholders::_1);
            m_Pool->Run(runFunc, selectedMove);
        }
    }
    selectedMove = MoveSlot::SLOT_NULL;

    m_Scene.update(deltaTime);

    //Update health
    healthA = health + std::to_string(m_Battle.getHealthA());
    healthB = health + std::to_string(m_Battle.getHealthB());

    //Update status
    conditionA = status + std::to_string(m_Battle.getStatusA());
    conditionB = status + std::to_string(m_Battle.getStatusB());

    //Update nickname
    nameA = m_Battle.getNameA();
    nameB = m_Battle.getNameB();

    //Update level
    if (first)
    {
        levelA = "Lv " + levelA;
        levelB = "Lv " + levelB;
    }
    first = false;
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