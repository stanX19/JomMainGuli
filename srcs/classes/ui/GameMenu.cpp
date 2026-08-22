#include "GameMenu.hpp"
#include "Engine.hpp"
#include "entities.hpp"
#include "ui/GameHUD.hpp"

GameMenu::GameMenu(GameContext &context)
	: m_context(context),
	  m_renderer(context.mainCamera, context),
	  m_startButton("START GAME", Rectangle{0.0f, 0.0f, 0.0f, 0.0f}, Color{56, 189, 248, 255}, 22),
	  m_helpButton("HOW TO PLAY & RECIPES", Rectangle{0.0f, 0.0f, 0.0f, 0.0f}, Color{148, 163, 184, 255}, 17)
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
	m_context.soundManager.playMusic();

	while (!WindowShouldClose() && nextState == EngineState::MENU)
	{
		const float dt = GetFrameTime();
		m_context.soundManager.updateMusic();

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

	const char *title = "JOM MAIN GULI";
	const int titleWidth = MeasureText(title, 54);
	DrawText(title, screenWidth / 2 - titleWidth / 2, screenHeight / 4, 54, Color{56, 189, 248, 255});

	const char *subTitle = "Malaysian Heritage Marble Sandbox";
	const int subWidth = MeasureText(subTitle, 20);
	DrawText(subTitle, screenWidth / 2 - subWidth / 2, screenHeight / 4 + 68, 20, Color{148, 163, 184, 255});

	const float btnWidth = 280.0f;
	const float btnHeight = 52.0f;
	const float startY = static_cast<float>(screenHeight) * 0.50f;

	const Rectangle btnStartRect = {static_cast<float>(screenWidth) * 0.5f - btnWidth * 0.5f, startY, btnWidth, btnHeight};
	m_startButton.setBounds(btnStartRect);

	const Rectangle btnHelpRect = {static_cast<float>(screenWidth) * 0.5f - btnWidth * 0.5f, startY + 66.0f, btnWidth, btnHeight};
	m_helpButton.setBounds(btnHelpRect);

	if (m_startButton.tickAndDraw())
	{
		nextState = EngineState::GAME;
	}

	if (m_helpButton.tickAndDraw())
	{
		m_showHelp = !m_showHelp;
	}

	const char *hint = "Press [ENTER] to Start   |   Press [H] for Recipes   |   Press [ESC] to Exit";
	const int hintWidth = MeasureText(hint, 15);
	DrawText(hint, screenWidth / 2 - hintWidth / 2, screenHeight - 45, 15, Color{148, 163, 184, 255});

	if (m_showHelp)
	{
		m_helpMenu.render(screenWidth, screenHeight);
	}
}

void GameMenu::inputControls([[maybe_unused]] float dt, EngineState &nextState)
{
	if (IsKeyPressed(KEY_H))
	{
		m_showHelp = !m_showHelp;
		return;
	}

	if (IsKeyPressed(KEY_ESCAPE))
	{
		if (m_showHelp)
		{
			m_showHelp = false;
			return;
		}
		nextState = EngineState::EXIT;
		return;
	}

	if (IsKeyPressed(KEY_ENTER))
	{
		nextState = EngineState::GAME;
	}
}

