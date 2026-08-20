#pragma once

#include "includes.hpp"

struct GameContext;

namespace ui {
	namespace palette {
		inline constexpr Color BG_PANEL = {15, 23, 42, 230};          // Deep Slate Navy
		inline constexpr Color BG_SELECTED = {30, 41, 59, 245};       // Selected Slate
		inline constexpr Color BORDER_DEFAULT = {71, 85, 105, 255};   // Slate Border
		inline constexpr Color BORDER_ACTIVE = {251, 191, 36, 255};   // Amber Gold
		inline constexpr Color BORDER_CYAN = {56, 189, 248, 255};     // Luminous Cyan
		inline constexpr Color TEXT_GOLD = {251, 191, 36, 255};       // Warm Gold
		inline constexpr Color TEXT_WHITE = {248, 250, 252, 255};     // Pure Off-White
		inline constexpr Color TEXT_MUTED = {203, 213, 225, 255};     // Slate Silver
		inline constexpr Color TEXT_CYAN = {56, 189, 248, 255};       // Luminous Teal
		inline constexpr Color TEXT_SUCCESS = {74, 222, 128, 255};    // Emerald Green
	}

	class GameHUD {
	public:
		GameHUD() = default;
		~GameHUD() = default;

		void render(const GameContext &context, int totalGuliInMap);

		static Vector2 getSlotPreviewCenter(size_t index, int screenWidth);
		static float getSlotSize() { return 64.0f; }
		static float getSlotSpacing() { return 70.0f; }


	private:
		void renderTopLeftHUD(size_t collectedCount, int winTarget);
		void renderTopRightInventory(const GameContext &context);
		void renderInspectionBanner(const GameContext &context);
		void renderVictoryBanner(size_t collectedCount, int winTarget);
	};
} // namespace ui

