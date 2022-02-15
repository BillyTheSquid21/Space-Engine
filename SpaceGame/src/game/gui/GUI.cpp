#include "game/gui/GUI.h"

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