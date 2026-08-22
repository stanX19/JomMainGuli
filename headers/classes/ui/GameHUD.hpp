#pragma once

#include "includes.hpp"

struct GameContext;

#include "ui/HelpMenu.hpp"

namespace ui {
	namespace palette {
		inline constexpr Color BG_PANEL = {15, 23, 42, 235};          // Deep Slate Obsidian (#0F172A)
		inline constexpr Color BG_SURFACE = {30, 41, 59, 240};        // Slate Charcoal (#1E293B)
		inline constexpr Color BG_BADGE = {15, 23, 42, 220};          // Corner badge background
		inline constexpr Color BORDER_SUBTLE = {51, 65, 85, 220};     // Slate Border (#334155)
		inline constexpr Color BORDER_ACTIVE = {56, 189, 248, 255};   // Luminous Sky Cyan (#38BDF8)
		inline constexpr Color BORDER_HERITAGE = {251, 191, 36, 255}; // Warm Amber Gold (#FBBF24)
		inline constexpr Color TEXT_GOLD = {251, 191, 36, 255};       // Warm Amber Gold (#FBBF24)
		inline constexpr Color TEXT_PRIMARY = {248, 250, 252, 255};   // Pure Frost White (#F8FAFC)
		inline constexpr Color TEXT_MUTED = {148, 163, 184, 255};     // Slate Silver (#94A3B8)
		inline constexpr Color TEXT_ACCENT = {56, 189, 248, 255};     // Sky Cyan (#38BDF8)
		inline constexpr Color TEXT_SUCCESS = {52, 211, 153, 255};    // Emerald Green (#34D399)
		inline constexpr Color TEXT_DIM = {100, 116, 139, 180};       // Muted Gray for Overtime (#64748B)
	}

	class GameHUD {
	public:
		GameHUD() = default;
		~GameHUD() = default;

		void render(GameContext &context, float dt);

		static void renderHelpModal(int screenWidth, int screenHeight);
		static Vector2 getSlotPreviewCenter(size_t index, int screenWidth);
		static float getSlotSize() { return 75.0f; }
		static float getSlotSpacing() { return 85.0f; }

	private:
		HelpMenu m_helpMenu;

		void renderFloatingStatus(const GameContext &context, size_t collectedCount, int winTarget);
		void renderTrapezoidClock(const GameContext &context);
		void renderToasts(GameContext &context, float dt);
		void renderInventorySidebar(const GameContext &context);
		void renderInspectionView(const GameContext &context);
	};
} // namespace ui

