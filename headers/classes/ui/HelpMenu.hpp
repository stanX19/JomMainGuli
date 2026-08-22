#pragma once

#include "includes.hpp"
#include <string>
#include <vector>

namespace ui {

	class HelpMenu {
	public:
		static constexpr float CHAR_ADVANCE_RATIO = 0.70f;

		HelpMenu(const std::string &filePath = "assets/help.txt", int fontSize = 20, float lineSpacing = 24.0f, float padding = 36.0f);
		~HelpMenu() = default;

		void load(const std::string &filePath);

		float getWidth() const;
		float getHeight() const;

		void render(int screenWidth, int screenHeight);
		void render(Vector2 position);

	private:
		std::vector<std::string> m_lines;
		float m_measuredWidth = 0.0f;
		float m_measuredHeight = 0.0f;
		const int m_fontSize = 20;
		const float m_lineSpacing = 24.0f;
		const float m_padding = 36.0f;

		void measureContent();
	};

} // namespace ui
