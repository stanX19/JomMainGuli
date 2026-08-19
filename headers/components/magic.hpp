#pragma once

#include "includes.hpp"

namespace component {
	struct MagicCastState {
		entt::entity graviton = entt::null;
		float castTimer = 0.0f;
		float spawnTimer = 0.0f;
		int spawnCount = 0;
	};

	struct MagicParticle {
		Color color = WHITE;
	};

	struct AttractedBy {
		entt::entity target = entt::null;
		float strength = 100.0f;
	};
} // namespace component
