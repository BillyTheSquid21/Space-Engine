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
    ImageDim dim = GetImageDimension("res/textures/pokemon/Bulbasaur.png");

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

void Battle::init(int width, int height, FontContainer* fonts, FlagArray* flags, GameInput* input) {
    //Width and height
    m_Width = width; m_Height = height;

    //Flags
    m_Flags = flags;

    //Input
    m_Input = input;

    //Renderer
    m_Renderer.initialiseRenderer(width, height);

    //Platform test
    m_Scene.init(width, height);

    //Gui test
    std::shared_ptr<GameGUI::DebugPanel> dp1(new GameGUI::DebugPanel(600.0f, 225.0f, 20.0f, 20.0f));
    gui1.setBase(dp1);
    std::shared_ptr<GameGUI::Divider> div1(new GameGUI::Divider());
    div1->m_FillY = false; div1->m_FillX = true;
    div1->m_Height = 72.0f;
    std::shared_ptr<GameGUI::TextBox> tex1(new GameGUI::TextBox(healthB));
    tex1->setNest(0);
    std::shared_ptr<GameGUI::TextBox> tex3(new GameGUI::TextBox(conditionB));
    tex3->setNest(0);
    gui1.addElement(div1);
    gui1.addElement(tex1);
    gui1.addElement(tex3);

    std::shared_ptr<GameGUI::DebugPanel> dp2(new GameGUI::DebugPanel(600.0f, 225.0f, 1300.0f, 850.0f));
    gui2.setBase(dp2);
    std::shared_ptr<GameGUI::Divider> div2(new GameGUI::Divider());
    div2->m_FillY = false; div2->m_FillX = true;
    div2->m_Height = 72.0f;
    std::shared_ptr<GameGUI::TextBox> tex2(new GameGUI::TextBox(healthA));
    tex2->setNest(0);
    std::shared_ptr<GameGUI::TextBox> tex4(new GameGUI::TextBox(conditionA));
    tex4->setNest(0);
    gui2.addElement(div2);
    gui2.addElement(tex2);
    gui2.addElement(tex4);
    std::shared_ptr<GameGUI::Divider> div3(new GameGUI::Divider());
    div3->m_FillY = false; div3->m_FillX = true;
    div3->m_Height = 120.0f;
    div3->setNest(1);
    gui2.addElement(div3);
    std::shared_ptr<GameGUI::Button> but(new GameGUI::Button("Next", &trigger));
    but->setNest(1);
    gui2.addElement(but);

    EngineLog("Battle scene loaded");
}

void Battle::loadRequiredData()
{
    m_PlayerParty[0].id = 0;
    m_PlayerParty[0].moves[0].type = PokemonType::Normal;
    m_PlayerParty[0].moves[0].damage = 25;
    m_EnemyParty[0].id = 0;
    m_EnemyParty[0].condition = StatusCondition::Paralysis;
    m_EnemyParty[0].moves[0].type = PokemonType::Normal;
    m_EnemyParty[0].moves[0].damage = 25;
    m_Battle.setParties(m_PlayerParty, m_EnemyParty);

    m_Renderer.loadRendererData();

    m_DataLoaded = true;
}

void Battle::purgeRequiredData()
{
    m_Renderer.purgeData();
}

void Battle::update(double deltaTime, double time)
{
    m_Battle.run(trigger);
    m_Scene.update(deltaTime);

    //Update health
    healthA = health + std::to_string(m_Battle.getHealthA());
    healthB = health + std::to_string(m_Battle.getHealthB());

    //Update status
    conditionA = status + std::to_string(m_Battle.getStatusA());
    conditionB = status + std::to_string(m_Battle.getStatusB());
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
    gui1.render();
    GameGUI::EndFrame();
    GameGUI::StartFrame();
    gui2.render();
    GameGUI::EndFrame();
}

void Battle::handleInput(int key, int scancode, int action, int mods)
{
    m_Input->handleInput(key, scancode, action, mods);
    if (m_Input->PRESSED_A)
    {
        trigger = true;
    }
    else
    {
        trigger = false;
    }
}