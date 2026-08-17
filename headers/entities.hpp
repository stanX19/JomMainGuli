#pragma once

#include "includes.hpp"
#include "components.hpp"
#include "game_context.hpp"

namespace entity {
	entt::entity spawnPlayer(GameContext &context);
	entt::entity spawnPlayer(GameContext &context, Vector3 pos);
} // namespace entity
