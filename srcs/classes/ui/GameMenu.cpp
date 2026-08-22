#include "GameMenu.hpp"
#include "Engine.hpp"
#include "entities.hpp"

GameMenu::GameMenu(GameContext &context)
	: m_context(context),
	  m_renderer(context.mainCamera, context),
	  m_startButton("START GAME", Rectangle{0.0f, 0.0f, 0.0f, 0.0f}, SKYBLUE, 26)
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

	const char *title = "JOM MAIN GULI!";
	const int titleWidth = MeasureText(title, 50);
	DrawText(title, screenWidth / 2 - titleWidth / 2, screenHeight / 8, 50, Color{251, 191, 36, 255});

	const char *subTitle = "Lets play glass marble!";
	const int subWidth = MeasureText(subTitle, 22);
	DrawText(subTitle, screenWidth / 2 - subWidth / 2, screenHeight / 8 + 58, 22, Color{56, 189, 248, 255});

	// How to play panel
	const float panelWidth = 780.0f;
	const float panelHeight = 230.0f;
	const Rectangle guideRect = {(float)screenWidth / 2 - panelWidth * 0.5f, (float)screenHeight / 8 + 95, panelWidth, panelHeight};
	DrawRectangleRounded(guideRect, 0.12f, 4, Color{15, 23, 42, 230});
	DrawRectangleRoundedLines(guideRect, 0.12f, 4, Color{71, 85, 105, 255});

	const char *guideTitle = "HOW TO PLAY";
	const int gTitleWidth = MeasureText(guideTitle, 22);
	DrawText(guideTitle, screenWidth / 2 - gTitleWidth / 2, static_cast<int>(guideRect.y + 16), 22, Color{251, 191, 36, 255});

	const float startY = guideRect.y + 52.0f;
	const float leftX = guideRect.x + 30.0f;
	DrawText("1. Collect Guli: Press and hold LMB near colored tiles", static_cast<int>(leftX), static_cast<int>(startY), 18, Color{248, 250, 252, 255});
	DrawText("2. Shoot: Release guli shots to kick big Guli", static_cast<int>(leftX), static_cast<int>(startY + 32), 18, Color{248, 250, 252, 255});
	DrawText("3. Win: Kick big Guli into holes!", static_cast<int>(leftX), static_cast<int>(startY + 64), 18, Color{248, 250, 252, 255});
	DrawText("4. Collect: Touch marbles to collect", static_cast<int>(leftX), static_cast<int>(startY + 96), 18, Color{248, 250, 252, 255});

	const Rectangle btnStart = {(float)screenWidth / 2 - 130, guideRect.y + panelHeight + 25, 260, 56};
	m_startButton.setBounds(btnStart);

	if (m_startButton.tickAndDraw())
	{
		nextState = EngineState::GAME;
	}

	const char *hint = "Press ESC to Exit";
	const int hintWidth = MeasureText(hint, 20);
	DrawText(hint, screenWidth / 2 - hintWidth / 2, screenHeight - 50, 20, Color{203, 213, 225, 255});
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
