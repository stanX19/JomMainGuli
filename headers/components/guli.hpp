#pragma once

#include "includes.hpp"

namespace component {
	namespace tags {
		struct GuliTarget {};
		struct CollectibleGuli {};
	}

	struct ShotGuli {
		Color color = WHITE;
	};

	struct MergingInHole {
		entt::entity guliEntity = entt::null;
		Vector3 targetPos = {0.0f, 0.0f, 0.0f};
	};
} // namespace component
