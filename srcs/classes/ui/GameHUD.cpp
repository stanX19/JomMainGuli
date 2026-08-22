#include "ui/GameHUD.hpp"
#include "GameContext.hpp"
#include "components.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace component;

Vector2 ui::GameHUD::getSlotPreviewCenter(size_t index, int screenWidth) {
	const float socketSize = getSlotSize();
	const float socketX = static_cast<float>(screenWidth) - socketSize - 20.0f;
	const float socketY = 20.0f + static_cast<float>(index) * getSlotSpacing();
	return Vector2{socketX + socketSize * 0.5f, socketY + socketSize * 0.5f};
}

void ui::GameHUD::renderTrapezoidClock(const GameContext &context) {
	const int screenWidth = GetScreenWidth();
	const float cx = static_cast<float>(screenWidth) * 0.5f;
	const float topHalfW = 140.0f;
	const float botHalfW = 105.0f;
	const float clockHeight = 56.0f;

	const Vector2 vTopLeft = {cx - topHalfW, 0.0f};
	const Vector2 vTopRight = {cx + topHalfW, 0.0f};
	const Vector2 vBotRight = {cx + botHalfW, clockHeight};
	const Vector2 vBotLeft = {cx - botHalfW, clockHeight};

	DrawTriangle(vTopLeft, vBotLeft, vTopRight, palette::BG_PANEL);
	DrawTriangle(vTopRight, vBotLeft, vBotRight, palette::BG_PANEL);

	DrawLineEx(vTopLeft, vBotLeft, 3.0f, palette::BORDER_SUBTLE);
	DrawLineEx(vBotLeft, vBotRight, 3.0f, palette::BORDER_SUBTLE);
	DrawLineEx(vBotRight, vTopRight, 3.0f, palette::BORDER_SUBTLE);

	const float timer = context.state.gameTimer;
	const bool isOvertime = (timer <= 0.0f);
	std::string timerStr;

	if (isOvertime) {
		timerStr = "0:00";
	} else {
		const int totalSec = static_cast<int>(timer);
		const int minutes = totalSec / 60;
		const int seconds = totalSec % 60;
		timerStr = (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" +
		           (seconds < 10 ? "0" : "") + std::to_string(seconds);
	}

	const Color clockColor = context.state.isVictory ? palette::TEXT_SUCCESS : (isOvertime ? palette::TEXT_DIM : palette::TEXT_PRIMARY);
	const int textWidth = MeasureText(timerStr.c_str(), 36);
	DrawText(timerStr.c_str(), static_cast<int>(cx - textWidth * 0.5f), 10, 36, clockColor);
}

void ui::GameHUD::renderToasts(GameContext &context, float dt) {
	if (context.state.toasts.empty())
		return;

	const int screenWidth = GetScreenWidth();
	const int screenHeight = GetScreenHeight();

	for (auto it = context.state.toasts.begin(); it != context.state.toasts.end(); ) {
		it->lifetime -= dt;
		if (it->lifetime <= 0.0f) {
			it = context.state.toasts.erase(it);
			continue;
		}

		const float alpha = std::min(1.0f, it->lifetime / 0.5f);
		const Color textColor = Fade(it->color, alpha);

		std::vector<std::string> lines;
		std::stringstream ss(it->text);
		std::string line;
		while (std::getline(ss, line, '\n')) {
			lines.push_back(line);
		}

		const float lineHeight = static_cast<float>(it->fontSize) + 8.0f;
		const float totalBlockHeight = static_cast<float>(lines.size()) * lineHeight;
		float currentY = static_cast<float>(screenHeight) * 0.45f - totalBlockHeight * 0.5f;

		for (const auto &l : lines) {
			const int textWidth = MeasureText(l.c_str(), it->fontSize);
			const int drawX = screenWidth / 2 - textWidth / 2;

			DrawText(l.c_str(), drawX + 2, static_cast<int>(currentY) + 2, it->fontSize, Fade(BLACK, alpha * 0.85f));
			DrawText(l.c_str(), drawX, static_cast<int>(currentY), it->fontSize, textColor);

			currentY += lineHeight;
		}

		++it;
	}
}

void ui::GameHUD::renderFloatingStatus(const GameContext &context, size_t collectedCount, int winTarget) {
	(void)context;
	DrawText("JOM MAIN GULI", 28, 24, 22, palette::TEXT_MUTED);

	const bool winMet = (static_cast<int>(collectedCount) >= winTarget);
	const std::string countStr = std::to_string(collectedCount) + " / " + std::to_string(winTarget);
	const int countWidth = MeasureText(countStr.c_str(), 48);

	DrawText(countStr.c_str(), 28, 48, 48, winMet ? palette::TEXT_SUCCESS : palette::TEXT_PRIMARY);
	DrawText("Guli Collected", 28 + countWidth + 16, 64, 24, palette::TEXT_MUTED);

	DrawText("[H] Help & Recipes", 28, GetScreenHeight() - 38, 24, palette::TEXT_MUTED);
}

void ui::GameHUD::renderInventorySidebar(const GameContext &context) {
	if (!context.registry.valid(context.currentPlayer))
		return;

	const auto *inventory = context.registry.try_get<GuliInventory>(context.currentPlayer);
	if (!inventory || inventory->guliCollection.empty())
		return;

	const auto *viewState = context.registry.try_get<GuliViewState>(context.currentPlayer);
	const int screenWidth = GetScreenWidth();

	const float socketSize = getSlotSize();
	const float socketX = static_cast<float>(screenWidth) - socketSize - 20.0f;

	for (size_t i = 0; i < inventory->guliCollection.size(); ++i) {
		const entt::entity guli = inventory->guliCollection[i];
		if (!context.registry.valid(guli))
			continue;

		const bool isSelected = (viewState && viewState->selectedIndex == i);
		const bool isSpecial = context.registry.all_of<tags::IsSpecialGuli>(guli);

		const float socketY = 20.0f + static_cast<float>(i) * getSlotSpacing();
		const Rectangle socketRect = {socketX, socketY, socketSize, socketSize};

		const Color borderColor = isSelected ? palette::BORDER_ACTIVE : (isSpecial ? palette::BORDER_HERITAGE : palette::BORDER_SUBTLE);
		DrawRectangleRoundedLines(socketRect, 0.22f, 4, borderColor);

		const std::string badgeText = std::to_string(i + 1);
		DrawText(badgeText.c_str(), static_cast<int>(socketX + 11.0f), static_cast<int>(socketY + 9.0f), 28, Color{0, 0, 0, 220});
		DrawText(badgeText.c_str(), static_cast<int>(socketX + 10.0f), static_cast<int>(socketY + 8.0f), 28, isSelected ? palette::TEXT_ACCENT : (isSpecial ? palette::TEXT_GOLD : palette::TEXT_PRIMARY));
	}
}

void ui::GameHUD::renderInspectionView(const GameContext &context) {
	if (!context.registry.valid(context.currentPlayer))
		return;

	const auto *viewState = context.registry.try_get<GuliViewState>(context.currentPlayer);
	if (!viewState || !context.registry.valid(viewState->targetGuli))
		return;

	const entt::entity target = viewState->targetGuli;
	const std::string name = context.registry.all_of<Description>(target)
		? context.registry.get<Description>(target).value
		: "Crafted Guli";

	const bool isSpecial = context.registry.all_of<tags::IsSpecialGuli>(target);
	const int screenWidth = GetScreenWidth();
	const int screenHeight = GetScreenHeight();

	const std::string header = (isSpecial ? "* " : "") + name;
	const int headerWidth = MeasureText(header.c_str(), 32);
	const float cardWidth = std::max(360.0f, static_cast<float>(headerWidth) + 60.0f);
	const float cardHeight = 60.0f;
	const float cardY = static_cast<float>(screenHeight) * 0.72f;
	const Rectangle cardRect = {static_cast<float>(screenWidth) * 0.5f - cardWidth * 0.5f, cardY, cardWidth, cardHeight};

	DrawRectangleRounded(cardRect, 0.2f, 4, palette::BG_PANEL);
	DrawRectangleRoundedLines(cardRect, 0.2f, 4, isSpecial ? palette::BORDER_HERITAGE : palette::BORDER_SUBTLE);

	DrawText(header.c_str(), screenWidth / 2 - headerWidth / 2, static_cast<int>(cardY + 14.0f), 32, isSpecial ? palette::TEXT_GOLD : palette::TEXT_PRIMARY);

	const char *navText = "[LMB + Drag] Rotate View   |   [ESC / RMB] Back";
	const int navTextWidth = MeasureText(navText, 24);
	DrawText(navText, screenWidth / 2 - navTextWidth / 2 + 1, screenHeight - 47, 24, Color{0, 0, 0, 200});
	DrawText(navText, screenWidth / 2 - navTextWidth / 2, screenHeight - 48, 24, palette::TEXT_MUTED);
}

void ui::GameHUD::renderHelpModal(int screenWidth, int screenHeight) {
	static HelpMenu sharedHelpMenu;
	sharedHelpMenu.render(screenWidth, screenHeight);
}

void ui::GameHUD::render(GameContext &context, float dt) {
	const int winTarget = std::min(5, std::max(1, context.state.totalGuliInMap));

	size_t collectedCount = 0;
	if (context.registry.valid(context.currentPlayer)) {
		if (const auto *inv = context.registry.try_get<GuliInventory>(context.currentPlayer)) {
			collectedCount = inv->guliCollection.size();
		}
	}

	renderTrapezoidClock(context);
	renderToasts(context, dt);
	renderFloatingStatus(context, collectedCount, winTarget);
	renderInventorySidebar(context);
	renderInspectionView(context);

	if (context.state.showHelpOverlay) {
		m_helpMenu.render(GetScreenWidth(), GetScreenHeight());
	}
}




