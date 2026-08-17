#pragma once

#include "includes.hpp"
#include "GameConfig.hpp"
#include "ModelManager.hpp"
#include "SoundManager.hpp"

struct GameContext {
	GameConfig config;
	ModelManager modelManager;
	SoundManager soundManager;
	entt::registry registry;
	entt::dispatcher dispatcher;
	entt::entity currentPlayer = entt::null;
	Camera3D mainCamera{};
};
