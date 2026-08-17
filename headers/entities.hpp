#pragma once

#include "includes.hpp"
#include "components.hpp"
#include "GameContext.hpp"

namespace entity {
	entt::entity spawnPlayer(GameContext &context);
	entt::entity spawnPlayer(GameContext &context, Vector3 pos);
	entt::entity spawnOrb(GameContext &context, Vector3 pos, Color color, float radius = 1.0f);
} // namespace entity
