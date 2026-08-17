#include "utils.hpp"
#include <algorithm>

Vector2 utils::input::getMouseRatioRelCenter()
{
	Vector2 mousePos = GetMousePosition();
	Vector2 screenCenter = Vector2{GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
	Vector2 relPos = mousePos - screenCenter;
	return relPos / screenCenter;
}

Vector2 utils::input::getMouseDirectionNormalized(float clampRatio)
{
	Vector2 mousePos = GetMousePosition();
	Vector2 screenCenter = Vector2{GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
	Vector2 relPos = mousePos - screenCenter;
	float clampDist = std::min(screenCenter.x, screenCenter.y) * clampRatio;
	return Vector2Normalize(relPos) * std::min(1.0f, Vector2Length(relPos) / clampDist);
}