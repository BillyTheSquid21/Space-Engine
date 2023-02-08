#include "core/GUI.h"

std::vector<SGGUI::System::GUIStorage> SGGUI::System::s_GUIList;
float SGGUI::System::s_XOff = 0.0f;
float SGGUI::System::s_YOff = 0.0f;
float SGGUI::System::s_Width = 1280.0f;
float SGGUI::System::s_Height = 720.0f;
bool SGGUI::System::s_Set = false;
int32_t SGGUI::System::s_NextGUIID = 1;

std::unique_ptr<SGGUI::FontMap> SGGUI::FontStorage::m_FontMap;

void SGGUI::System::set()
{
	clean();
	s_Set = true;
}

void SGGUI::System::clean()
{
	if (!s_Set)
	{
		return;
	}

	s_GUIList.clear();
	s_NextGUIID = 1;
	s_Set = false;
}

int32_t SGGUI::System::addGUI(std::shared_ptr<GUIBase> gui)
{
	if (!s_Set)
	{
		return -1;
	}

	int32_t id = s_NextGUIID;
	s_GUIList.emplace_back(id, gui, false);

	s_NextGUIID++;
	return id;
}

bool SGGUI::System::removeGUI(int32_t id)
{
	if (!s_Set)
	{
		return false;
	}

	for (int i = 0; i < s_GUIList.size(); i++)
	{
		if (s_GUIList[i].id == id)
		{
			s_GUIList.erase(s_GUIList.begin() + i);
			return true;
		}
	}
	return false;
}

void SGGUI::System::setShowGUI(int32_t id, bool show)
{
	if (!s_Set)
	{
		return;
	}

	for (auto& gui : s_GUIList)
	{
		if (gui.id == id)
		{
			gui.show = show;
			return;
		}
	}
}

bool SGGUI::System::accessGUI(int32_t id, GUIBase** gui)
{
	if (!s_Set)
	{
		return false;
	}

	for (int i = 0; i < s_GUIList.size(); i++)
	{
		if (s_GUIList[i].id == id)
		{
			*gui = s_GUIList[i].ptr.get();
			return true;
		}
	}
	return false;
}

void SGGUI::System::start()
{
	if (!s_Set)
	{
		return;
	}

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	s_XOff = s_Width * 0.025f; s_YOff = s_Height * 0.025f;
	ImGui::SetNextWindowSize(ImVec2(s_Width + 2.0f*s_XOff, s_Height + 2.0f*s_YOff), 0);
	ImGui::SetNextWindowPos(ImVec2(-s_XOff, -s_YOff), 0);
	ImGui::Begin("HUD", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
}

void SGGUI::System::end()
{
	if (!s_Set)
	{
		return;
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SGGUI::System::loadDefaultFonts()
{
	constexpr int sizeTotal = 8;
	int sizes[sizeTotal] =
	{
		8, 12, 18, 24, 36, 48, 56, 72
	};

	const char* menuFont = "res/fonts/FiraCode/FiraCode-SemiBold.ttf";
	for (int i = 0; i < sizeTotal; i++)
	{
		FontStorage::loadFont(menuFont, "menu", sizes[i]);
	}
}

void SGGUI::System::render()
{
	pushDefault();
	start();

	for (auto& gui : s_GUIList)
	{
		if (!gui.show)
		{
			continue;
		}
		gui.ptr->start(s_XOff, s_YOff, s_Width, s_Height);
		gui.ptr->end();
	}

	end();
	popDefault();
}

#define DEFAULT_STYLE_COUNT 3
#define DEFAULT_COLOR_COUNT 5

void SGGUI::System::pushDefault()
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

void SGGUI::System::popDefault()
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

void SGGUI::FontStorage::loadFont(const char* path, const char* fontName, unsigned char ptSize) {
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

ImFont* SGGUI::FontStorage::getFont(const char* fontName, unsigned char ptSize) {
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

void SGGUI::FontStorage::clear() {
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->ClearFonts();

	//Clear maps
	m_FontMap->clear();
}