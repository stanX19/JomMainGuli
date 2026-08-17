#pragma once

#include "GameContext.hpp"
#include "Renderer.hpp"
#include "TextButtonWidget.hpp"

enum class EngineState;

class GameMenu {
public:
	GameMenu(GameContext &context);
	~GameMenu();

	EngineState run();

private:
	GameContext &m_context;
	Renderer m_renderer;
	ui::TextButtonWidget m_startButton;

	void drawMenuUI(EngineState &nextState);
	void inputControls([[maybe_unused]] float dt, EngineState &nextState);
};
