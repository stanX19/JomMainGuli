#pragma once

#include "GameContext.hpp"
#include "Renderer.hpp"
#include "TextButtonWidget.hpp"
#include "ui/HelpMenu.hpp"

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
	ui::TextButtonWidget m_helpButton;
	ui::HelpMenu m_helpMenu;
	bool m_showHelp = false;

	void drawMenuUI(EngineState &nextState);
	void inputControls([[maybe_unused]] float dt, EngineState &nextState);
};
