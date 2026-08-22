#pragma once

#include "includes.hpp"
#include <string>
#include <vector>

enum class ToastPriority {
	LOW = 0,
	NORMAL = 1,
	HIGH = 2
};

struct ToastMessage {
	std::string text;
	Color color = WHITE;
	float lifetime = 3.0f;
	float maxLifetime = 3.0f;
	int fontSize = 18;
	ToastPriority priority = ToastPriority::NORMAL;
};

struct GameState {
	float gameTimer = 180.0f;
	bool isVictory = false;
	bool showHelpOverlay = false;
	int totalGuliInMap = 5;
	std::vector<ToastMessage> toasts;
	float cheatToastCooldown = 0.0f;
	float canShowStrikeToast = 0.0f;

	GameState(int totalGuli = 5, float timer = 180.0f)
		: gameTimer(timer),
		  isVictory(false),
		  showHelpOverlay(false),
		  totalGuliInMap(totalGuli),
		  toasts(),
		  cheatToastCooldown(0.0f),
		  canShowStrikeToast(0.0f)
	{}

	void addToast(const std::string &text, Color color, float duration = 3.0f, int fontSize = 18, ToastPriority priority = ToastPriority::NORMAL) {
		toasts.push_back(ToastMessage{text, color, duration, duration, fontSize, priority});
	}
};
