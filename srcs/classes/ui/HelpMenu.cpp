#include "ui/HelpMenu.hpp"
#include "ui/GameHUD.hpp"
#include <algorithm>
#include <fstream>

ui::HelpMenu::HelpMenu(const std::string &filePath, int fontSize, float lineSpacing, float padding)
	: m_fontSize(fontSize),
	  m_lineSpacing(lineSpacing),
	  m_padding(padding)
{
	load(filePath);
}

void ui::HelpMenu::load(const std::string &filePath) {
	m_lines.clear();
	std::ifstream file(filePath);
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			m_lines.push_back(line);
		}
	}
	measureContent();
}

void ui::HelpMenu::measureContent() {
	size_t maxChars = 0;
	for (const auto &line : m_lines) {
		if (line.size() > maxChars) {
			maxChars = line.size();
		}
	}

	const float charAdvance = static_cast<float>(m_fontSize) * CHAR_ADVANCE_RATIO;
	m_measuredWidth = static_cast<float>(maxChars) * charAdvance + m_padding * 2.0f;
	m_measuredHeight = static_cast<float>(m_lines.size()) * m_lineSpacing + m_padding * 2.0f;
}

float ui::HelpMenu::getWidth() const {
	return m_measuredWidth;
}

float ui::HelpMenu::getHeight() const {
	return m_measuredHeight;
}

void ui::HelpMenu::render(int screenWidth, int screenHeight) {
	DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 190});

	const float posX = static_cast<float>(screenWidth) * 0.5f - m_measuredWidth * 0.5f;
	const float posY = static_cast<float>(screenHeight) * 0.5f - m_measuredHeight * 0.5f;

	render(Vector2{posX, posY});
}

void ui::HelpMenu::render(Vector2 position) {
	const Rectangle modalRect = {position.x, position.y, m_measuredWidth, m_measuredHeight};

	DrawRectangleRounded(modalRect, 0.08f, 4, palette::BG_PANEL);
	DrawRectangleRoundedLines(modalRect, 0.08f, 4, palette::BORDER_ACTIVE);

	float drawY = position.y + m_padding;
	const float drawX = position.x + m_padding;
	const float charAdvance = static_cast<float>(m_fontSize) * CHAR_ADVANCE_RATIO;

	for (const auto &line : m_lines) {
		for (size_t col = 0; col < line.size(); ++col) {
			const char c = line[col];
			if (c == ' ' || c == '\t')
				continue;

			const char str[2] = {c, '\0'};
			const int charW = MeasureText(str, m_fontSize);
			const float cellX = drawX + static_cast<float>(col) * charAdvance + (charAdvance - static_cast<float>(charW)) * 0.5f;

			DrawText(str, static_cast<int>(cellX), static_cast<int>(drawY), m_fontSize, palette::TEXT_PRIMARY);
		}
		drawY += m_lineSpacing;
	}
}
