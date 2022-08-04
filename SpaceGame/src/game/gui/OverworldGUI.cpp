#include "game/gui/OverworldGUI.h"

//Overworld menu
void OverworldMenu::openNest()
{
	if (!m_Show)
	{
		return;
	}
	menuStyle();
	if (m_ShowPkmMenu)
	{
		pkmMenu();
	}
	else if (m_ShowBag)
	{
		bag();
	}
	Divider::openNest();
	ImGui::PushFont(m_Fonts->getFont("boxfont", 45));
	if (ImGui::Button("Pokemon", ImVec2(ImGui::GetContentRegionAvail().x, 75.0f)))
	{
		m_ShowPkmMenu = !m_ShowPkmMenu;
		m_ShowBag = false;
		unloadAllIcons();
	}
	if (ImGui::Button("Bag", ImVec2(ImGui::GetContentRegionAvail().x, 75.0f)))
	{
		m_ShowBag = !m_ShowBag;
		m_ShowPkmMenu = false;
		unloadAllIcons();
	}
	endMenuStyle();
}

void OverworldMenu::menuStyle()
{
	//Style
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 118.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 40.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 4.0f);

	//Color
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 0.9f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.36f, 0.4f, 0.45f, 0.85f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.996f, 0.694f, 0.224f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.996f, 0.794f, 0.324f, 1.0f));
}

void OverworldMenu::endMenuStyle()
{
	//Style
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	//Color
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void OverworldMenu::pkmMenu()
{
	pkmMenuStyle();
	ImGui::BeginChild("PkmMenu", ImVec2(m_Width * 5.2f, ImGui::GetContentRegionAvail().y), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	
	//Top divider
	ImGui::PushFont(m_Fonts->getFont("boxfont", 45));
	for (int i = 0; i < 6; i++)
	{
		pkmDivider(i);
	}
	ImGui::PopFont();
	ImGui::EndChild();
	endPkmMenuStyle();
}

void OverworldMenu::pkmDivider(int slot)
{
	//Check pokemon in slot
	if (m_PlayerData->playerParty[slot].id != -1)
	{
		//Check if fainted and should display red
		bool fainted = false;
		if (m_PlayerData->playerParty[slot].health <= 0)
		{
			fainted = true;
		}

		if (!fainted)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.227f, 0.606f, 0.849f, 1.0f));
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.82f, 0.32f, 0.22f, 1.0f));
		}

		//Create text - will probably make more efficient if needed
		std::string text = "";
		text += m_PlayerData->playerParty[slot].nickname;
		text += " Lv: " + std::to_string(m_PlayerData->playerParty[slot].level) + "\n";
		text[0] = std::toupper(text[0]);
		text += "Health: " + std::to_string(m_PlayerData->playerParty[slot].health) + "\n";
		text += "Status: " + std::to_string((int)m_PlayerData->playerParty[slot].condition);

		ImGui::Button(text.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, m_Height / 6.2f));
		ImGui::PopStyleColor();
	}
}

void OverworldMenu::pkmMenuStyle()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 36.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.01f, 0.25f));
}

void OverworldMenu::endPkmMenuStyle()
{
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void OverworldMenu::bag()
{
	ImGui::BeginChild("Bag", ImVec2(m_Width * 5.2f, ImGui::GetContentRegionAvail().y), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	bagButtons();

	//Add view
	float paddingX = ImGui::GetContentRegionAvail().x / 100.0f;
	float paddingY = ImGui::GetContentRegionAvail().y / 100.0f;

	bagInnerStyle();
	bagColor();
	ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x / 8.0f); ImGui::SetCursorPosY(2*paddingY);
	ImGui::BeginChild("BagView", ImVec2(ImGui::GetContentRegionAvail().x - paddingX, ImGui::GetContentRegionAvail().y - paddingY), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	bagItems();
	ImGui::EndChild();
	endBagColor();
	endBagInnerStyle();

	ImGui::EndChild();
}

void OverworldMenu::bagInnerStyle()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
}

void OverworldMenu::endBagInnerStyle()
{
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void OverworldMenu::getBagIconPath(std::string& path, uint16_t id)
{
	switch (id)
	{
	case 1:
		path += "MasterBall.png";
		return;
	case 2:
		path += "UltraBall.png";
		return;
	case 3:
		path += "GreatBall.png";
		return;
	case 4:
		path += "PokeBall.png";
		return;
	case 5:
		path += "SafariBall.png";
		return;
	case 6:
		path += "NetBall.png";
		return;
	case 7:
		path += "DiveBall.png";
		return;
	case 8:
		path += "NestBall.png";
		return;
	case 9:
		path += "RepeatBall.png";
		return;
	case 17:
		path += "Potion.png";
		return;
	case 23:
		path += "FullRestore.png";
		return;
	case 24:
		path += "MaxPotion.png";
		return;
	case 25:
		path += "HyperPotion.png";
		return;
	case 26:
		path += "SuperPotion.png";
		return;
	default:
		path += "Revive.png";
		return;
	}
}

void OverworldMenu::loadBagIcon(Texture& texture, uint16_t id)
{
	std::string path = "res/textures/icons/";
	getBagIconPath(path, id);
	texture.deleteTexture();
	texture.clearBuffer();
	texture.loadTexture(path, false);
	texture.generateTexture(0);
	m_IconsLoaded = true;
}

void OverworldMenu::unloadAllIcons()
{
	for (int i = 0; i < 12; i++)
	{
		m_Icons[i].deleteTexture();
		m_Icons[i].clearBuffer();
	}
	m_LastScroll = -1.0f; //Lets it know to reset
}

void OverworldMenu::bagItems()
{
	if (m_LastScroll != ImGui::GetScrollY() || m_LastPocket != m_Pocket)
	{
		m_AwaitingIconLoad = true;
		m_LastScroll = ImGui::GetScrollY();
		m_LastPocket = m_Pocket;
	}

	Item item; bool startIcons = false; int iconIndex = 0;
	for (int i = 0; i < m_PlayerData->bag.items.size(); i++)
	{
		item = m_PlayerData->bag.items[i].item;
		if (item.pocketId != (int)m_Pocket || m_PlayerData->bag.items[i].count < 1)
		{
			continue;
		}
		std::string name = item.name;
		name[0] = std::toupper(name[0]);
		float y = ImGui::GetCursorPosY();
		float x = ImGui::GetCursorPosX() + 155.0f;

		//If images unloaded, work out what image should be starting
		if (iconIndex > 11)
		{
			iconIndex = 0;
			startIcons = false;
			m_AwaitingIconLoad = false;
		}
		if (!startIcons && m_LastScroll < y + 300.0f)
		{
			startIcons = true;
			if (m_AwaitingIconLoad)
			{
				loadBagIcon(m_Icons[0], item.id);
			}
		}
		else if (startIcons)
		{
			if (m_AwaitingIconLoad)
			{
				loadBagIcon(m_Icons[iconIndex], item.id);
			}
		}

		if (startIcons)
		{
			ImGui::Image((void*)m_Icons[iconIndex].getID(), ImVec2(150.0f, 150.0f));
			iconIndex++;
		}
		ImGui::SetCursorPosX(x);
		ImGui::SetCursorPosY(y);
		ImGui::Button(name.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 150.0f));
	}
}

void OverworldMenu::bagColor()
{
	//Bag color
	switch(m_Pocket)
	{
	case BagPocket::Items:
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.35f, 1.0f));
		return;
	case BagPocket::Medicine:
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.28f, 0.22f, 1.0f));
		return;
	case BagPocket::Berries:
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.27f, 0.18f, 0.25f, 1.0f));
		return;
	case BagPocket::Pokeballs:
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.28f, 0.2f, 0.27f, 1.0f));
		return;
	case BagPocket::TMs:
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.25f, 0.30f, 1.0f));
		return;
	case BagPocket::KeyItems:
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.22f, 0.24f, 1.0f));
		return;
	default:
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.35f, 1.0f));
		return;
	}
}

void OverworldMenu::endBagColor()
{
	ImGui::PopStyleColor();
}

void OverworldMenu::bagButtons()
{
	//Add tabs
	ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y / 15.0f);

	//Items
	if (m_Pocket == BagPocket::Items)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.996f, 0.694f, 0.224f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.996f, 0.794f, 0.324f, 1.0f));
	}
	if (ImGui::Button("Items", ImVec2(ImGui::GetContentRegionAvail().x / 7.0f, 75.0f)) && m_Pocket != BagPocket::Items)
	{
		m_Pocket = BagPocket::Items;
		m_JustClicked = true;
	}
	if (m_Pocket == BagPocket::Items && !m_JustClicked)
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	//Medicine
	if (m_Pocket == BagPocket::Medicine)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.996f, 0.694f, 0.224f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.996f, 0.794f, 0.324f, 1.0f));
	}
	if (ImGui::Button("Medicine", ImVec2(ImGui::GetContentRegionAvail().x / 7.0f, 75.0f)) && m_Pocket != BagPocket::Medicine)
	{
		m_Pocket = BagPocket::Medicine;
		m_JustClicked = true;
	}
	if (m_Pocket == BagPocket::Medicine && !m_JustClicked)
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	//Balls
	if (m_Pocket == BagPocket::Pokeballs)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.996f, 0.694f, 0.224f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.996f, 0.794f, 0.324f, 1.0f));
	}
	if (ImGui::Button("Pokeballs", ImVec2(ImGui::GetContentRegionAvail().x / 7.0f, 75.0f)) && m_Pocket != BagPocket::Pokeballs)
	{
		m_Pocket = BagPocket::Pokeballs;
		m_JustClicked = true;
	}
	if (m_Pocket == BagPocket::Pokeballs && !m_JustClicked)
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	//Berries
	if (m_Pocket == BagPocket::Berries)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.996f, 0.694f, 0.224f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.996f, 0.794f, 0.324f, 1.0f));
	}
	if (ImGui::Button("Berries", ImVec2(ImGui::GetContentRegionAvail().x / 7.0f, 75.0f)) && m_Pocket != BagPocket::Berries)
	{
		m_Pocket = BagPocket::Berries;
		m_JustClicked = true;
	}
	if (m_Pocket == BagPocket::Berries && !m_JustClicked)
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	//TMs
	if (m_Pocket == BagPocket::TMs)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.996f, 0.694f, 0.224f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.996f, 0.794f, 0.324f, 1.0f));
	}
	if (ImGui::Button("TMs", ImVec2(ImGui::GetContentRegionAvail().x / 7.0f, 75.0f)) && m_Pocket != BagPocket::TMs)
	{
		m_Pocket = BagPocket::TMs;
		m_JustClicked = true;
	}
	if (m_Pocket == BagPocket::TMs && !m_JustClicked)
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	//Key items
	if (m_Pocket == BagPocket::KeyItems)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.996f, 0.694f, 0.224f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.996f, 0.794f, 0.324f, 1.0f));
	}
	if (ImGui::Button("Key Items", ImVec2(ImGui::GetContentRegionAvail().x / 7.0f, 75.0f)) && m_Pocket != BagPocket::KeyItems)
	{
		m_Pocket = BagPocket::KeyItems;
		m_JustClicked = true;
	}
	if (m_Pocket == BagPocket::KeyItems && !m_JustClicked)
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}
	m_JustClicked = false;
}

void OverworldMenu::closeNest()
{
	if (!m_Show)
	{
		return;
	}
	ImGui::PopFont();
	Divider::closeNest();
}