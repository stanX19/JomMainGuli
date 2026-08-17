#include "GameMenu.hpp"
#include "Engine.hpp"
#include "entities.hpp"

GameMenu::GameMenu(GameContext &context)
	: m_context(context),
	  m_renderer(context.mainCamera, context),
	  m_startButton("START GAME", Rectangle{0.0f, 0.0f, 0.0f, 0.0f}, SKYBLUE, 20)
{
}

GameMenu::~GameMenu() = default;

EngineState GameMenu::run()
{
	const float arenaSize = m_context.config.ARENA_SIZE;
	m_context.mainCamera.position = Vector3{arenaSize * 0.1f, arenaSize * 0.1f, arenaSize * 0.1f};
	m_context.mainCamera.target = Vector3{0.0f, 0.0f, 0.0f};
	m_context.mainCamera.up = Vector3{0.0f, 1.0f, 0.0f};
	m_context.mainCamera.fovy = 45.0f;
	m_context.mainCamera.projection = CAMERA_PERSPECTIVE;

	EngineState nextState = EngineState::MENU;

	while (!WindowShouldClose() && nextState == EngineState::MENU)
	{
		const float dt = GetFrameTime();

		BeginDrawing();
		ClearBackground(BLACK);
		m_renderer.render(dt);
		drawMenuUI(nextState);
		EndDrawing();

		inputControls(dt, nextState);
	}
	return nextState;
}

void GameMenu::drawMenuUI(EngineState &nextState)
{
	const int screenWidth = GetScreenWidth();
	const int screenHeight = GetScreenHeight();

	const char *title = "CODESFAIRES";
	const int titleWidth = MeasureText(title, 50);
	DrawText(title, screenWidth / 2 - titleWidth / 2, screenHeight / 3, 50, SKYBLUE);

	const Rectangle btnStart = {(float)screenWidth / 2 - 100, (float)screenHeight / 2, 200, 50};
	m_startButton.setBounds(btnStart);

	if (m_startButton.tickAndDraw())
	{
		nextState = EngineState::GAME;
	}

	const char *hint = "Press ESC to Exit";
	const int hintWidth = MeasureText(hint, 20);
	DrawText(hint, screenWidth / 2 - hintWidth / 2, screenHeight * 3 / 4, 20, GRAY);
}

void GameMenu::inputControls([[maybe_unused]] float dt, EngineState &nextState)
{
	if (IsKeyPressed(KEY_ESCAPE))
	{
		nextState = EngineState::EXIT;
		return;
	}
	if (IsKeyPressed(KEY_ENTER))
	{
		nextState = EngineState::GAME;
	}
}
