#pragma once

#include "includes.hpp"
#include "map/Map.hpp"
#include <optional>

namespace map {
	class MapCollider {
	public:
		static std::optional<Vector3> resolveSphere(
			const Map &map,
			Vector3 &pos,
			float radius
		);
	};
} // namespace map
