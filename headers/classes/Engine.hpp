#pragma once

#include "GameContext.hpp"
#include "Game.hpp"
#include "GameMenu.hpp"

enum class EngineState {
	MENU,
	GAME,
	EXIT
};

class Engine {
public:
	Engine();
	~Engine();

	void run();

private:
	GameContext m_context;
	EngineState m_state = EngineState::MENU;

	void init();
	void shutdown();
};
