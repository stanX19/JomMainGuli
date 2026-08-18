#pragma once

#include "includes.hpp"
#include <cstddef>
#include <cstdint>

namespace component {
	struct SpawnsTrailParticles {
		float radius = 0.5f;
		float lifespan = 1.0f;
		Color color = SKYBLUE;
	};

	struct Trail {
		Color color = WHITE;
		float rad = 0.1f;
		Trail(Color c = WHITE, float r = 0.1f) : color(c), rad(r) {}
	};

	struct RadiusExpand {
		float speed = 0.0f; // radius += speed * dt
	};

	struct Lifespan {
		float value = 1.0f;
	};
} // namespace component
