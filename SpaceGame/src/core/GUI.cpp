#include "core/GUI.h"

float GUI::GameGUI::m_XOff = 0.0f;
float GUI::GameGUI::m_YOff = 0.0f;

std::unique_ptr<GUI::FontMap> GUI::FontContainer::m_FontMap;

void GUI::GameGUI::GUIStart(int width, int height)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	m_XOff = width * 0.025f; m_YOff = height * 0.025f;
	ImGui::SetNextWindowSize(ImVec2(width + 2.0f*m_XOff, height + 2.0f*m_YOff), 0);
	ImGui::SetNextWindowPos(ImVec2(-m_XOff, -m_YOff), 0);
	ImGui::Begin("HUD", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
}

void GUI::GameGUI::GUIEnd()
{
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::GameGUI::LoadDefaultFonts()
{
	constexpr int sizeTotal = 8;
	int sizes[sizeTotal] =
	{
		8, 12, 18, 24, 36, 48, 56, 72
	};

	const char* menuFont = "res/fonts/FiraCode/FiraCode-SemiBold.ttf";
	for (int i = 0; i < sizeTotal; i++)
	{
		FontContainer::loadFont(menuFont, "menu", sizes[i]);
	}
}

#define DEFAULT_STYLE_COUNT 3
#define DEFAULT_COLOR_COUNT 5

void GUI::GameGUI::PushDefault()
{
	//Style
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

	//Color
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(PALETTE2_BLACK, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(PALETTE1_BLACK_CORAL, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(PALETTE1_LAUREL_GREEN, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(PALETTE1_CAMBRIDGE_BLUE, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(PALETTE1_GREEN_SHEEN, 1.0f));
}

void GUI::GameGUI::PopDefault()
{
	for (int i = 0; i < DEFAULT_STYLE_COUNT; i++)
	{
		ImGui::PopStyleVar();
	}
	for (int i = 0; i < DEFAULT_COLOR_COUNT; i++)
	{
		ImGui::PopStyleColor();
	}
}

void GUI::FontContainer::loadFont(const char* path, const char* fontName, unsigned char ptSize) {
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(path, ptSize);
	//Check if font style has been loaded

	if (m_FontMap->find(fontName) == m_FontMap->end()) {
		FontSizeMap map;
		(*m_FontMap)[fontName].map = map;
		(*m_FontMap)[fontName].path = path;
	}

	//loads map
	FontSizeMap sizeMap = m_FontMap->at(fontName).map;
	if (sizeMap.find(ptSize) != sizeMap.end()) {
		EngineLog("Font already loaded:", ptSize);
		return;
	}
	//Adds font pointer to map and returns to font map
	sizeMap[ptSize] = io.Fonts->Fonts.Size - 1;
	(*m_FontMap)[fontName].map = sizeMap;
}

ImFont* GUI::FontContainer::getFont(const char* fontName, unsigned char ptSize) {
	ImGuiIO& io = ImGui::GetIO();	
	//Get index from map
	if (m_FontMap->find(fontName) == m_FontMap->end()) {
		//Defaults to returning default font
		EngineLog("Font style not found:", fontName);
		return io.Fonts->Fonts[0];
	}
	//If not, loads next map
	FontSMap_FontPath sizeMap = m_FontMap->at(fontName);
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

void GUI::FontContainer::clear() {
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->ClearFonts();

	//Clear maps
	m_FontMap->clear();
}