#pragma once

#include "includes.hpp"
#include "map/Map.hpp"
#include <optional>
#include <unordered_map>

namespace map {
	class MapCollider {
	public:
		static std::optional<Vector3> calculateSphereCollisionNormals(
			const Map &map,
			Vector3 &pos,
			const Vector3 &prevPos,
			float radius
		);
		static std::optional<Vector3> calculateSphereCollisionNormals(
			const Map &map,
			Vector3 &pos,
			float radius
		);
		static std::vector<TileCollisionData> collideTilesInRange(
			const Map &map,
			const Vector3 &pos,
			const Vector3 &prevPos,
			float radius
		);
		// cell, collision coordinate
		static std::vector<TileCollisionData> collideTilesInRange(
			const Map &map,
			Vector3 &pos,
			float radius
		);
	};
} // namespace map
