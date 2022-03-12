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
void GameGUI::TextBox::setStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 18.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 16.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.29f, 0.29f, 0.35f, 0.95f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.22f, 0.25f, 1.00f);
}

void GameGUI::TextBox::run(std::string& text1, std::string& text2)
{
	renderStart();

	ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	ImGui::PushFont(m_Fonts->getFont("boxfont", 70));
	ImGui::BeginChild("##Centre", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false);
	ImGui::SetCursorPosX(40.0f);
	ImGui::SetCursorPosY(40.0f);
	ImGui::Text(text1.c_str());
	ImGui::SetCursorPosX(40.0f);
	ImGui::SetCursorPosY(135.0f);
	ImGui::Text(text2.c_str());
	ImGui::PopFont();
	ImGui::EndChild();
	
	renderEnd();
}