#pragma once

#include "includes.hpp"

namespace component {
	struct MagicCastState {
		Vector3 focusPoint = {0.0f, 0.0f, 0.0f};
		float castTimer = 0.0f;
		float spawnTimer = 0.0f;
	};

	struct MagicParticle {
		Color color = WHITE;
	};

	struct CastedParticle {
		entt::entity caster = entt::null;
	};
} // namespace component
