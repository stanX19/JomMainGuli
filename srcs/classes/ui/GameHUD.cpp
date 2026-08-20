#include "ui/GameHUD.hpp"
#include "GameContext.hpp"
#include "components.hpp"
#include <algorithm>
#include <string>

using namespace component;

Vector2 ui::GameHUD::getSlotPreviewCenter(size_t index, int screenWidth) {
	const float socketSize = getSlotSize();
	const float socketX = static_cast<float>(screenWidth) - socketSize;
	const float socketY = 20.0f + static_cast<float>(index) * getSlotSpacing();
	return Vector2{socketX + socketSize * 0.5f, socketY + socketSize * 0.5f};
}

void ui::GameHUD::renderTopLeftHUD(size_t collectedCount, int winTarget) {
	const float panelWidth = std::max(400.0f, 60.0f + static_cast<float>(winTarget) * 60.0f);
	const float panelHeight = 115.0f;
	const Rectangle panelRect = {15.0f, 15.0f, panelWidth, panelHeight};

	DrawRectangleRounded(panelRect, 0.12f, 4, palette::BG_PANEL);
	DrawRectangleRoundedLines(panelRect, 0.12f, 4, palette::BORDER_DEFAULT);

	DrawText("Jom Main Guli", 28, 24, 22, palette::TEXT_GOLD);

	for (int k = 1; k <= winTarget; ++k) {
		const float boxX = 28.0f + static_cast<float>(k - 1) * 58.0f;
		const float boxY = 52.0f;
		const Rectangle boxRect = {boxX, boxY, 50.0f, 30.0f};

		const bool isFilled = (k <= static_cast<int>(collectedCount));
		DrawRectangleRounded(boxRect, 0.2f, 4, isFilled ? palette::BG_SELECTED : Fade(BLACK, 0.50f));
		DrawRectangleRoundedLines(boxRect, 0.2f, 4, isFilled ? palette::BORDER_ACTIVE : palette::BORDER_DEFAULT);

		const std::string numStr = std::to_string(k);
		const int textWidth = MeasureText(numStr.c_str(), 18);
		DrawText(numStr.c_str(), static_cast<int>(boxX + 25.0f - textWidth * 0.5f), static_cast<int>(boxY + 6.0f), 18, isFilled ? palette::TEXT_GOLD : palette::TEXT_MUTED);
	}

	const bool winMet = (static_cast<int>(collectedCount) >= winTarget);
	const std::string progressText = "Collected: " + std::to_string(collectedCount) + " / " + std::to_string(winTarget) + "  |  [1-9] Inspect";
	DrawText(progressText.c_str(), 28, 88, 16, winMet ? palette::TEXT_SUCCESS : palette::TEXT_WHITE);
}

void ui::GameHUD::renderTopRightInventory(const GameContext &context) {
	if (!context.registry.valid(context.currentPlayer))
		return;

	const auto *inventory = context.registry.try_get<GuliInventory>(context.currentPlayer);
	if (!inventory || inventory->guliCollection.empty())
		return;

	const auto *viewState = context.registry.try_get<GuliViewState>(context.currentPlayer);
	const int screenWidth = GetScreenWidth();

	const float socketSize = getSlotSize();
	const float socketX = static_cast<float>(screenWidth) - 65.0f;

	for (size_t i = 0; i < inventory->guliCollection.size(); ++i) {
		const entt::entity guli = inventory->guliCollection[i];
		if (!context.registry.valid(guli))
			continue;

		const bool isSelected = (viewState && viewState->selectedIndex == i);
		const bool isSpecial = context.registry.all_of<tags::IsSpecialGuli>(guli);

		const float socketY = 20.0f + static_cast<float>(i) * getSlotSpacing();
		const Rectangle socketRect = {socketX, socketY, socketSize, socketSize};

		// const Color bgColor = isSelected ? palette::BG_SELECTED : palette::BG_PANEL;
		const Color borderColor = isSelected ? palette::BORDER_ACTIVE : (isSpecial ? palette::BORDER_ACTIVE : palette::BORDER_DEFAULT);

		// DrawRectangleRounded(socketRect, 0.25f, 4, bgColor);
		DrawRectangleRoundedLines(socketRect, 0.25f, 4, borderColor);
	}
}

void ui::GameHUD::renderInspectionBanner(const GameContext &context) {
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

	const float bannerY = static_cast<float>(screenHeight) * 0.42f + 115.0f;
	const Rectangle bannerRect = {static_cast<float>(screenWidth) * 0.5f - 240.0f, bannerY, 480.0f, 75.0f};

	DrawRectangleRounded(bannerRect, 0.18f, 4, Fade(BLACK, 0.85f));
	DrawRectangleRoundedLines(bannerRect, 0.18f, 4, isSpecial ? palette::BORDER_ACTIVE : palette::BORDER_DEFAULT);

	const std::string header = (isSpecial ? "* " : "") + name;
	const int headerWidth = MeasureText(header.c_str(), 22);
	DrawText(header.c_str(), screenWidth / 2 - headerWidth / 2, static_cast<int>(bannerY + 12.0f), 22, isSpecial ? palette::TEXT_GOLD : palette::TEXT_WHITE);

	const std::string fullControls = "Hold LMB & Drag to Rotate";
	const int ctrlWidth = MeasureText(fullControls.c_str(), 15);
	DrawText(fullControls.c_str(), screenWidth / 2 - ctrlWidth / 2, static_cast<int>(bannerY + 44.0f), 15, palette::TEXT_MUTED);
}

void ui::GameHUD::renderVictoryBanner(size_t collectedCount, int winTarget) {
	if (winTarget <= 0 || static_cast<int>(collectedCount) < winTarget)
		return;

	const int screenWidth = GetScreenWidth();
	const Rectangle winRect = {static_cast<float>(screenWidth) * 0.5f - 320.0f, 15.0f, 640.0f, 85.0f};

	DrawRectangleRounded(winRect, 0.18f, 4, palette::BG_PANEL);
	DrawRectangleRoundedLines(winRect, 0.18f, 4, palette::BORDER_ACTIVE);

	const char *winTitle = "VICTORY! GAME COMPLETE!";
	const int titleWidth = MeasureText(winTitle, 24);
	DrawText(winTitle, screenWidth / 2 - titleWidth / 2, 24, 24, palette::TEXT_GOLD);

	const std::string subTitle = "All Required Gulies Collected (" + std::to_string(collectedCount) + " / " + std::to_string(winTarget) + ")  |  [R] Replay";
	const int subWidth = MeasureText(subTitle.c_str(), 16);
	DrawText(subTitle.c_str(), screenWidth / 2 - subWidth / 2, 54, 16, palette::TEXT_WHITE);
}

void ui::GameHUD::render(const GameContext &context, int totalGuliInMap) {
	const int winTarget = std::min(5, std::max(1, totalGuliInMap));

	size_t collectedCount = 0;
	if (context.registry.valid(context.currentPlayer)) {
		if (const auto *inv = context.registry.try_get<GuliInventory>(context.currentPlayer)) {
			collectedCount = inv->guliCollection.size();
		}
	}

	renderTopLeftHUD(collectedCount, winTarget);
	renderTopRightInventory(context);
	renderInspectionBanner(context);
	renderVictoryBanner(collectedCount, winTarget);
}
