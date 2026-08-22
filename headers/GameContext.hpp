#pragma once

#include "includes.hpp"
#include "GameConfig.hpp"
#include "GameState.hpp"
#include "ModelManager.hpp"
#include "SoundManager.hpp"
#include "map/Map.hpp"

struct GameContext {
	GameConfig config;
	GameState state;
	ModelManager modelManager;
	SoundManager soundManager;
	map::Map map;
	entt::registry registry;
	entt::dispatcher dispatcher;
	entt::entity currentPlayer = entt::null;
	Camera3D mainCamera{};
};
