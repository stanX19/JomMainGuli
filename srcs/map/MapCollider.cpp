#include "constants.hpp"
#include "map/MapCollider.hpp"
#include <algorithm>
#include <cmath>

std::optional<Vector3> map::MapCollider::resolveSphere(
	const Map &map,
	Vector3 &pos,
	float radius
) {
	if (map.getWidth() <= 0 || map.getHeight() <= 0 || radius <= 0.0f)
		return std::nullopt;

	const CellCord minCord = map.worldToGrid(Vector3{pos.x - radius, 0.0f, pos.z - radius});
	const CellCord maxCord = map.worldToGrid(Vector3{pos.x + radius, 0.0f, pos.z + radius});

	const int startX = std::max(0, minCord.x);
	const int endX = std::min(map.getWidth() - 1, maxCord.x);
	const int startY = std::max(0, minCord.y);
	const int endY = std::min(map.getHeight() - 1, maxCord.y);

	bool collided = false;
	Vector3 totalNormal = {0.0f, 0.0f, 0.0f};

	// for each candidate cell, find the closest point within cell's box to the sphere's center.
	// if closest dist < rad, collided
	// can accumulate
	for (int y = startY; y <= endY; ++y) {
		for (int x = startX; x <= endX; ++x) {
			const TileData &tile = map.getTile(x, y);

			const float clampedX = std::clamp(pos.x, tile.x1, tile.x2);
			const float clampedY = std::clamp(pos.y, -100.0f, tile.selfHeight);
			const float clampedZ = std::clamp(pos.z, tile.z1, tile.z2);
			const Vector3 closestPoint = {clampedX, clampedY, clampedZ};

			const Vector3 pointToSphere = pos - closestPoint;
			const float distSq = Vector3LengthSqr(pointToSphere);

			if (distSq < radius * radius) {
				collided = true;
				const float dist = std::sqrt(distSq);
				const Vector3 normal = (dist > constants::epsilon) ? (pointToSphere / dist) : Vector3{0.0f, 1.0f, 0.0f};
				const float penetration = radius - dist;

				pos += normal * penetration;
				totalNormal += normal;
			}
		}
	}

	if (!collided)
		return std::nullopt;

	if (Vector3LengthSqr(totalNormal) > constants::epsilon) {
		return Vector3Normalize(totalNormal);
	}

	return Vector3{0.0f, 1.0f, 0.0f};
}
