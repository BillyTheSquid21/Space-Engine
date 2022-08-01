#include "game/gui/BattleGUI.h"

void BattleGUI::openNest()
{
	std::lock_guard lock(m_AccessMutex);
	guiStyle();
	ImGui::PushFont(m_Fonts->getFont("boxfont", 45));

	//Child boxes
	enemyBox();
	if (PokemonBattle::textBuffer.line1 != "")
	{
		textBox();
	}

	//Main player box
	Divider::openNest();
	playerBox();

	endGuiStyle();
}

void BattleGUI::playerBox()
{
	ImVec2 maxSize = ImGui::CalcTextSize("0000000000000000");

	//Name and lvl
	ImGui::SetCursorPosX(25.0f);
	ImGui::SetCursorPosY(15.0f);
	ImGui::Text(m_PkmNameA.c_str());
	ImGui::SetCursorPosX(40.0f + maxSize.x);
	ImGui::SetCursorPosY(15.0f);
	ImGui::Text(m_PkmLvlA.c_str());

	//progress bar
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.12f, 0.62f, 0.12f, 1));
	ImGui::SetCursorPosX(20.0f);
	ImGui::ProgressBar(m_HealthPercA, ImVec2(ImGui::GetContentRegionAvail().x - 12.0f, 50.0f));
	ImGui::PopStyleColor();

	//Each button
	ImGui::SetCursorPosX(20.0f);
	ImGui::SetCursorPosY(15.0f + maxSize.y + 60.0f);
	float width = ImGui::GetContentRegionAvail().x / 2.1f;
	float height = ImGui::GetContentRegionAvail().y / 3.9f;

	if (m_TriggersCount > 0)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, m_ButtonColors[0]);
		if (ImGui::Button(m_MoveNames[0].c_str(), ImVec2(width, height)))
		{
			m_MoveTriggers[0] = true;
		}
		else
		{
			m_MoveTriggers[0] = false;
		}
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::Button(" ", ImVec2(width, height));
	}

	ImGui::SetCursorPosY(15.0f + maxSize.y + 60.0f);
	ImGui::SetCursorPosX(20.0f + width + 15.0f);
	if (m_TriggersCount > 1)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, m_ButtonColors[1]);
		if (ImGui::Button(m_MoveNames[1].c_str(), ImVec2(width, height)))
		{
			m_MoveTriggers[1] = true;
		}
		else
		{
			m_MoveTriggers[1] = false;
		}
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::Button(" ", ImVec2(width, height));
	}

	ImGui::SetCursorPosX(20.0f);
	ImGui::SetCursorPosY(15.0f + maxSize.y + height + 10.0f + 60.0f);
	if (m_TriggersCount > 2)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, m_ButtonColors[2]);
		if (ImGui::Button(m_MoveNames[2].c_str(), ImVec2(width, height)))
		{
			m_MoveTriggers[2] = true;
		}
		else
		{
			m_MoveTriggers[2] = false;
		}
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::Button(" ", ImVec2(width, height));
	}

	ImGui::SetCursorPosY(15.0f + maxSize.y + height + 10.0f + 60.0f);
	ImGui::SetCursorPosX(20.0f + width + 15.0f);
	if (m_TriggersCount > 3)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, m_ButtonColors[3]);
		if (ImGui::Button(m_MoveNames[3].c_str(), ImVec2(width, height)))
		{
			m_MoveTriggers[3] = true;
		}
		else
		{
			m_MoveTriggers[3] = false;
		}
		ImGui::PopStyleColor();
	}
	else
	{
		ImGui::Button(" ", ImVec2(width, height));
	}

	//Other selection
	float width2 = ImGui::GetContentRegionAvail().x / 3.25f;
	ImGui::SetCursorPosY(15.0f + maxSize.y + height + height + 20.0f + 60.0f);
	ImGui::SetCursorPosX(20.0f);
	ImGui::Button("Bag", ImVec2(width2, ImGui::GetContentRegionAvail().y));
	ImGui::SetCursorPosY(15.0f + maxSize.y + height + height + 20.0f + 60.0f);
	ImGui::SetCursorPosX(20.0f + width2 + 10.0f);
	ImGui::Button("Run", ImVec2(width2, ImGui::GetContentRegionAvail().y));
	ImGui::SetCursorPosY(15.0f + maxSize.y + height + height + 20.0f + 60.0f);
	ImGui::SetCursorPosX(20.0f + width2 + width2 + 20.0f);
	if (ImGui::Button("Pokemon", ImVec2(width2, ImGui::GetContentRegionAvail().y)))
	{
		*m_SwitchPkm = true;
	}
	else
	{
		*m_SwitchPkm = false;
	}
}

void BattleGUI::enemyBox()
{
	ImGui::BeginChild("enemyBox", ImVec2(m_Width, m_Height/2.5f), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	ImVec2 maxSize = ImGui::CalcTextSize("0000000000000000");

	//Name and lvl
	ImGui::SetCursorPosX(25.0f);
	ImGui::SetCursorPosY(15.0f);
	ImGui::Text(m_PkmNameB.c_str());
	ImGui::SetCursorPosX(40.0f + maxSize.x);
	ImGui::SetCursorPosY(15.0f);
	ImGui::Text(m_PkmLvlB.c_str());

	//progress bar
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.12f, 0.62f, 0.12f, 1));
	ImGui::SetCursorPosX(20.0f);
	ImGui::ProgressBar(m_HealthPercB, ImVec2(ImGui::GetContentRegionAvail().x - 12.0f, 50.0f));
	ImGui::PopStyleColor();

	ImGui::EndChild();
}

void BattleGUI::textBox()
{
	ImGui::SetCursorPosX(m_XPos - (m_Width*1.2f) - 10.0f);
	ImGui::SetCursorPosY(m_YPos);
	ImGui::BeginChild("textBox", ImVec2(m_Width*1.2f, m_Height/2.0f), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetCursorPosX(10.0f);
	ImGui::SetCursorPosY(15.0f);
	ImGui::Text(PokemonBattle::textBuffer.line1.c_str());
	ImGui::Text(PokemonBattle::textBuffer.line2.c_str());
	ImGui::EndChild();
}

void BattleGUI::closeNest()
{
	ImGui::PopFont();
	Divider::closeNest();
}

void BattleGUI::guiStyle()
{
	//Style
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 118.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 40.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.35f));

	//Color
	ImGui::PushStyleColor(ImGuiCol_ChildBg, WINDOW_COLOR);
	ImGui::PushStyleColor(ImGuiCol_Button, BUTTON_COLOR);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 1));
}

void BattleGUI::endGuiStyle()
{
	//Style
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	//Color
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void BattleGUI::setMoves(std::array<PokemonMove, 4>& moves)
{
	m_TriggersCount = 0;
	m_MoveNames[0] = " "; m_MoveNames[1] = " ";
	m_MoveNames[2] = " "; m_MoveNames[3] = " ";
	for (int i = 0; i < 4; i++)
	{
		if (moves[i].id == 0)
		{
			m_ButtonColors[i] = BUTTON_COLOR;
			continue;
		}

		m_MoveNames[i] = moves[i].identifier;
		m_MoveNames[i][0] = std::toupper(m_MoveNames[i][0]);
		m_ButtonColors[i] = getMoveColor(moves[i].type);

		m_TriggersCount++;
	}
}

ImVec4 BattleGUI::getMoveColor(PokemonType type)
{
	switch (type)
	{
	case PokemonType::Normal:
		return ImVec4(0.451f, 0.467f, 0.482f, 1.0f);
	case PokemonType::Fire:
		return ImVec4(0.937f, 0.357f, 0.047f, 1.0f);
	case PokemonType::Water:
		return ImVec4(0.0f, 0.588f, 1.0f, 1.0f);
	case PokemonType::Grass:
		return ImVec4(0.49f, 0.808f, 0.075f, 1.0f);
	case PokemonType::Bug:
		return ImVec4(0.918f, 0.898f, 0.035f, 1.0f);
	case PokemonType::Dark:
		return ImVec4(0.173f, 0.2f, 0.2f, 1.0f);
	case PokemonType::Dragon:
		return ImVec4(0.686f, 0.478f, 0.702f, 1.0f);
	case PokemonType::Electric:
		return ImVec4(1.0f, 0.761f, 0.235f, 1.0f);
	case PokemonType::Fairy:
		return ImVec4(1.0f, 0.561f, 0.694f, 1.0f);
	case PokemonType::Fighting:
		return ImVec4(0.6f, 0.0f, 0.01f, 1.0f);
	case PokemonType::Flying:
		return ImVec4(0.651f, 0.82f, 0.902f, 1.0f);
	case PokemonType::Ghost:
		return ImVec4(0.38f, 0.047f, 0.388f, 1.0f);
	case PokemonType::Ground:
		return ImVec4(0.792f, 0.584f, 0.361f, 1.0f);
	case PokemonType::Ice:
		return ImVec4(0.739f, 0.837f, 0.829f, 1.0f);
	case PokemonType::Poison:
		return ImVec4(0.663f, 0.063f, 0.475f, 1.0f);
	case PokemonType::Psychic:
		return ImVec4(0.98f, 0.306f, 0.671f, 1.0f);
	case PokemonType::Rock:
		return ImVec4(0.643f, 0.494f, 0.231f, 1.0f);
	case PokemonType::Steel:
		return ImVec4(0.812f, 0.824f, 0.812f, 1.0f);
	default:
		return BUTTON_COLOR;
	}
}