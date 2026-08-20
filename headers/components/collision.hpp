#pragma once

#include "includes.hpp"

namespace component {
	namespace tags {
		struct GlassCollisionSound {};
		using glassCollisionSound = GlassCollisionSound;
	} // namespace tags

	struct CollisionBody {
		float radius = 1.0f;
	};
} // namespace component
