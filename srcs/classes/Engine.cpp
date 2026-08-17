#include "Engine.hpp"

Engine::Engine() {
	init();
}

Engine::~Engine() {
	shutdown();
}

void Engine::init() {
	InitWindow(1600, 900, "CodeSfaires");
	SetTargetFPS(60);
	SetExitKey(KEY_NULL);

	m_context.soundManager.init(m_context.config);
}

void Engine::shutdown() {
	m_context.soundManager.shutdown();
	m_context.modelManager.unloadAll();
	CloseWindow();
}

void Engine::run() {
	GameMenu menu(m_context);
	Game game(m_context);

	while (m_state != EngineState::EXIT) {
		if (WindowShouldClose()) {
			m_state = EngineState::EXIT;
			break;
		}

		switch (m_state) {
			case EngineState::MENU:
				m_state = menu.run();
				if (m_state == EngineState::GAME)
					game.reset();
				break;
			case EngineState::GAME:
				m_state = game.run();
				break;
			case EngineState::EXIT:
				break;
		}
	}
}
