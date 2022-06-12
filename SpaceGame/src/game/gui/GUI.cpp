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
void GameGUI::GUIContainer::render()
{
	m_Base->setStyle();
	m_Base->renderStart();
	m_Base->openNest();
	
	for (int i = 0; i < m_NestCount + 1; i++)
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

void GameGUI::Divider::openNest()
{
	float width = m_Width; float height = m_Height;
	if (m_FillX)
	{
		width = ImGui::GetContentRegionAvail().x;
	}
	if (m_FillY)
	{
		height = ImGui::GetContentRegionAvail().y;
	}

	ImGui::BeginChild(m_Name.c_str(), ImVec2(width, height), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
}

void GameGUI::Divider::closeNest()
{
	ImGui::EndChild();
}

void GameGUI::DebugPanel::openNest()
{
	ImGui::Begin("Debug Menu", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	ImGuiIO& io = ImGui::GetIO();
	ImGui::PushFont(io.Fonts->Fonts[0]);
}

void GameGUI::DebugPanel::closeNest()
{
	ImGui::PopFont();
}

void GameGUI::TextBox::openNest()
{
	ImGui::Text(m_Text.c_str());
}

GameGUI::GameTextBox::GameTextBox(float width, float height, float x, float y, std::string& t1, std::string& t2)
	: m_Text1Ref(t1), m_Text2Ref(t2)
{
	m_WindowWidth = width; m_WindowHeight = height; m_WindowX = x; m_WindowY = y;
}

void GameGUI::GameTextBox::openNest()
{
	ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	ImGui::PushFont(m_Fonts->getFont("boxfont", 70));
	ImGui::BeginChild("##Centre", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false);
	ImGui::SetCursorPosX(40.0f);
	ImGui::SetCursorPosY(40.0f);
	ImGui::Text(m_Text1Ref.c_str());
	ImGui::SetCursorPosX(40.0f);
	ImGui::SetCursorPosY(135.0f);
	ImGui::Text(m_Text2Ref.c_str());
}

void GameGUI::GameTextBox::closeNest()
{
	ImGui::PopFont();
	ImGui::EndChild();
}