#include "game/gui/GUI.h"

void GameGUI::StartFrame()
{
	ImGui_ImplOpenGL3_NewFrame(); 
	ImGui_ImplGlfw_NewFrame(); 
	ImGui::NewFrame(); 
}

void GameGUI::EndFrame()
{
	ImGui::Render(); 
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 
}

void GameGUI::SetColors(int r, int g, int b, ImGuiCol target) {
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[target] = ImColor(r, g, b);
}

void GameGUI::SetNextWindowSize(float width, float height) {
	ImGui::SetNextWindowSize(ImVec2(width, height), 0);
}

void GameGUI::SetNextWindowPos(float x, float y) {
	ImGui::SetNextWindowPos(ImVec2(x, y), 0);
}

void GameGUI::ResetStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowPadding = ImVec2(10, 10);
	style->WindowRounding = 8.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 6.0f;
	style->ChildRounding = 8.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.29f, 0.29f, 0.35f, 0.95f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.22f, 0.25f, 1.00f);
}

void FontContainer::loadFont(const char* path, const char* fontName, unsigned char ptSize) {
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(path, ptSize);

	//Check if font style has been loaded
	if (m_FontMap.find(fontName) == m_FontMap.end()) {
		FontSizeMap map;
		m_FontMap[fontName].map = map;
		m_FontMap[fontName].path = path;
	}

	//loads map
	FontSizeMap sizeMap = m_FontMap.at(fontName).map;
	if (sizeMap.find(ptSize) != sizeMap.end()) {
		//Defaults to returning default font
		EngineLog("Font already loaded:", ptSize);
		return;
	}
	//Adds font pointer to map and returns to font map
	sizeMap[ptSize] = io.Fonts->Fonts.Size - 1;
	m_FontMap[fontName].map = sizeMap;
}

ImFont* FontContainer::getFont(const char* fontName, unsigned char ptSize) {
	ImGuiIO& io = ImGui::GetIO();
	//Get index from map
	if (m_FontMap.find(fontName) == m_FontMap.end()) {
		//Defaults to returning default font
		EngineLog("Font style not found:", fontName);
		return io.Fonts->Fonts[0];
	}
	//If not, loads next map
	FontSMap_FontPath sizeMap = m_FontMap.at(fontName);
	if (sizeMap.map.find(ptSize) == sizeMap.map.end()) {
		//Defaults to returning default font
		EngineLog("Font size not found", (unsigned int)ptSize);
		//Returns this for first pass
		return io.Fonts->Fonts[0];
	}
	//If not, returns pointer
	unsigned int index = sizeMap.map.at(ptSize);
	return io.Fonts->Fonts[index];
}

void FontContainer::clearFonts() {
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->ClearFonts();

	//Clear maps
	m_FontMap.clear();
}

//GUI
void GameGUI::GUIElement::updateDimensions()
{
	if (m_FillX)
	{
		m_Width = ImGui::GetContentRegionAvail().x;
	}
	if (m_FillY)
	{
		m_Height = ImGui::GetContentRegionAvail().y;
	}
	if (m_XPos != -1.0f)
	{
		ImGui::SetCursorPosX(m_XPos);
	}
	if (m_YPos != -1.0f)
	{
		ImGui::SetCursorPosY(m_YPos);
	}
}

void GameGUI::GUIContainer::showNest(int nest, bool show)
{
	for (int i = 0; i < m_Elements.size(); i++)
	{
		if (m_Elements[i]->getNest() == nest)
		{
			m_Elements[i]->m_Show = show;
		}
	}
}

void GameGUI::GUIContainer::render()
{
	if (!m_Base)
	{
		return;
	}
	m_Base->setStyle();
	m_Base->renderStart();
	m_Base->openNest();
	for (int i = 0; i <= m_NestCount; i++)
	{
		//Open all nests (and close self contained)
		for (int j = 0; j < m_Elements.size(); j++)
		{
			if (m_Elements[j]->getNest() != i || m_Elements[j]->m_RenderCycleComplete)
			{
				continue;
			}

			//Open all nests in order
			m_Elements[j]->openNest();
			if (m_Elements[j]->m_SelfContained)
			{
				m_Elements[j]->closeNest();
				m_Elements[j]->m_RenderCycleComplete = true;
			}
		}
		//Close all unclosed nests in reverse order
		for (int j = m_Elements.size() - 1; j >= 0; j--)
		{
			if (m_Elements[j]->getNest() != i || m_Elements[j]->m_RenderCycleComplete)
			{
				continue;
			}
			
			m_Elements[j]->closeNest();
			m_Elements[j]->m_RenderCycleComplete = true;
		}
	}
	m_Base->closeNest();
	m_Base->renderEnd();

	//Reset cycle count
	for (int i = 0; i < m_Elements.size(); i++)
	{
		m_Elements[i]->m_RenderCycleComplete = false;
	}
}

//HUD
void GameGUI::HUD::openNest()
{
	ImGui::Begin("Debug Menu", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
	ImGuiIO& io = ImGui::GetIO();
	ImGui::PushFont(io.Fonts->Fonts[0]);
}

void GameGUI::HUD::closeNest()
{
	ImGui::PopFont();
}

void GameGUI::Divider::openNest()
{
	if (!m_Show)
	{
		return;
	}
	updateDimensions();
	ImGui::BeginChild(m_Name.c_str(), ImVec2(m_Width, m_Height), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
}

void GameGUI::Divider::closeNest()
{
	if (!m_Show)
	{
		return;
	}
	ImGui::EndChild();
}

void GameGUI::TextBox::openNest()
{
	if (!m_Show)
	{
		return;
	}
	ImGui::Text(m_Text.c_str());
}

void GameGUI::Button::openNest()
{
	if (!m_Show)
	{
		return;
	}
	updateDimensions();
	if (ImGui::Button(m_ButtonText.c_str(), ImVec2(m_Width, m_Height))) {
		*m_Trigger = true;
	}
	else
	{
		*m_Trigger = false;
	}
}

void GameGUI::TextDisplay::openNest()
{
	if (!m_Show)
	{
		return;
	}
	Divider::openNest();
	ImGui::PushFont(m_Fonts->getFont("boxfont", 70));
	ImGui::SetCursorPosX(40.0f);
	ImGui::SetCursorPosY(40.0f);
	ImGui::Text(m_Text1Ref.c_str());
	ImGui::SetCursorPosX(40.0f);
	ImGui::SetCursorPosY(135.0f);
	ImGui::Text(m_Text2Ref.c_str());
}

void GameGUI::TextDisplay::closeNest()
{
	if (!m_Show)
	{
		return;
	}
	ImGui::PopFont();
	ImGui::EndChild();
}

//Text box buffer
void GameGUI::TextBuffer::pushBuffer(std::string text)
{
	//Allocate new characters
	m_Buffer.resize(m_Buffer.size() + text.length());
	auto bufferIterator = m_Buffer.end() - text.length();
	std::copy(text.begin(), text.end(), bufferIterator);
	m_IsReady = false;
}

void GameGUI::TextBuffer::nextPage()
{
	if (!m_NextPageReady)
	{
		return;
	}

	const int CHARACTER_BUFF_SIZE = 120;
	const int ESCAPE_MULTIPLIER = 10000;

	//Strip top 120 characters from buffer
	char characters[CHARACTER_BUFF_SIZE] = {};
	if (m_Buffer.size() < CHARACTER_BUFF_SIZE)
	{
		std::copy(m_Buffer.begin(), m_Buffer.end(), characters);
	}
	else
	{
		std::copy(m_Buffer.begin(), m_Buffer.begin() + CHARACTER_BUFF_SIZE-1, characters);
	}
	//Note locations of whitespaces in first 60 characters
	int spaces[CHARACTER_BUFF_SIZE] = {}; //Stack to max value of 120
	std::fill_n(spaces, CHARACTER_BUFF_SIZE, CHARACTER_BUFF_SIZE+1);
	int spacesIndex = 0;

	//Init spaces[0] to length of buffer in case only one word exists
	spaces[0] = m_Buffer.size();
	for (int i = 0; i < CHARACTER_BUFF_SIZE; i++)
	{
		if (characters[i] == ' ')
		{
			spaces[spacesIndex] = i;
			spacesIndex++;
		}
		else if (characters[i] == '\n')
		{
			characters[i] = 1;			   //ascii character 1 to signal to remove this character
			spaces[spacesIndex] = i*ESCAPE_MULTIPLIER; //*10000 signals \n so can recover original value
			spacesIndex++;
		}
	}

	//Work out how much to strip out for first line
	int endIndex1 = m_Buffer.size();
	int lastSpacesIndex = 0;
	for (int i = 0; i < CHARACTER_BUFF_SIZE; i++)
	{
		//Check if next index is beyond 60 mark
		if (spaces[i] > 60 && spaces[i] < ESCAPE_MULTIPLIER)
		{
			lastSpacesIndex = i;
			break;
		}
		else if (spaces[i] >= ESCAPE_MULTIPLIER)
		{
			spaces[i] = spaces[i]/ESCAPE_MULTIPLIER;
			endIndex1 = spaces[i];
			lastSpacesIndex = i;
			break;
		}
		//Otherwise current index is valid
		endIndex1 = spaces[i];
	}

	//Work out how much to strip out for second line
	int endIndex2 = 0;
	for (int i = lastSpacesIndex; i < CHARACTER_BUFF_SIZE; i++)
	{
		//Find last index
		if (spaces[i] == CHARACTER_BUFF_SIZE+1)
		{
			if (i == 0)
			{
				break;
			}
			endIndex2 = spaces[i - 1];
			break;
		}
		else if (spaces[i] >= ESCAPE_MULTIPLIER)
		{
			if (i == 0)
			{
				break;
			}
			endIndex2 = spaces[i] / ESCAPE_MULTIPLIER;
			break;
		}
	}

	//Fill strings with characters
	std::string line1tmp = "";
	for (int i = 0; i < endIndex1; i++)
	{
		if (characters[i] == 1)
		{
			continue;
		}
		line1tmp += characters[i];
	}
	std::string line2tmp = "";
	for (int i = endIndex1; i < endIndex2; i++)
	{
		if (i < 0 || i >= CHARACTER_BUFF_SIZE)
		{
			continue;
		}
		else if (characters[i] == 1)
		{
			continue;
		}
		line2tmp += characters[i];
	}

	//Remove used characters from buffer - check aren't erasing past end (can happen if a trailing \n is lost)
	int deleteIndex = endIndex2 + 1;
	if (deleteIndex >= m_Buffer.size())
	{
		m_Buffer.clear();
	}
	else
	{
		m_Buffer.erase(m_Buffer.begin(), m_Buffer.begin() + deleteIndex);
	}
	line1 = line1tmp; line2 = line2tmp;
	if (m_Buffer.size() <= 1)
	{
		m_IsReady = true;
	}
}