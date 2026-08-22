#include "TextButtonWidget.hpp"

namespace ui {

TextButtonWidget::TextButtonWidget(const std::string &text, Rectangle bounds, Color color, int fontSize)
	: m_text(text), m_bounds(bounds), m_color(color), m_fontSize(fontSize) {
}

void TextButtonWidget::setText(const std::string &newText) {
	m_text = newText;
}

void TextButtonWidget::setBounds(Rectangle newBounds) {
	m_bounds = newBounds;
}

void TextButtonWidget::setColor(Color newColor) {
	m_color = newColor;
}

bool TextButtonWidget::update() {
	m_hovered = CheckCollisionPointRec(GetMousePosition(), m_bounds);
	m_down = m_hovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
	return m_hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void TextButtonWidget::draw() {
	const Color bgColor = m_hovered ? ColorAlpha(m_color, 0.35f) : ColorAlpha(m_color, 0.18f);
	DrawRectangleRounded(m_bounds, 0.22f, 4, bgColor);
	DrawRectangleRoundedLines(m_bounds, 0.22f, 4, m_hovered ? Color{56, 189, 248, 255} : m_color);

	const int textWidth = MeasureText(m_text.c_str(), m_fontSize);
	DrawText(
		m_text.c_str(),
		static_cast<int>(m_bounds.x + m_bounds.width * 0.5f - textWidth * 0.5f),
		static_cast<int>(m_bounds.y + m_bounds.height * 0.5f - m_fontSize * 0.5f),
		m_fontSize,
		m_hovered ? Color{248, 250, 252, 255} : m_color
	);
}

} // namespace ui
