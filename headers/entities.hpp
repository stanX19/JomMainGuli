#pragma once

#include "includes.hpp"
#include "components.hpp"
#include "GameContext.hpp"

namespace entity {
	entt::entity spawnPlayer(GameContext &context);
	entt::entity spawnPlayer(GameContext &context, Vector3 pos);
	entt::entity spawnOrb(GameContext &context, Vector3 pos, Color color, float radius = 1.0f);
	entt::entity spawnGuli(GameContext &context, Vector3 pos, float radius = 1.0f);
	entt::entity spawnMagicParticle(GameContext &context, Vector3 pos, Color color, entt::entity target = entt::null, float radius = 0.4f);
} // namespace entity
