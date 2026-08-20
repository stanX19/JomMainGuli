#pragma once

#include "includes.hpp"
#include "map/MapTypes.hpp"
#include <string>

namespace component {
	namespace tags {
		struct GuliTarget {};
		struct CollectibleGuli {};
		struct IsSpecialGuli {};
	}

	struct Description {
		std::string value;
	};

	struct ShotGuli {
		Color color = WHITE;
		map::ColorType colorType = map::ColorType::White;
	};

	struct MergingInHole {
		entt::entity guliEntity = entt::null;
		Vector3 targetPos = {0.0f, 0.0f, 0.0f};
	};
} // namespace component
