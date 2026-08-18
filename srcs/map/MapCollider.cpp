#include "constants.hpp"
#include "utils.hpp"
#include "map/MapCollider.hpp"
#include <algorithm>
#include <cmath>

std::vector<map::TileCollisionData> map::MapCollider::collideTilesInRange(
	const Map &map,
	Vector3 &pos,
	float radius
) {
	std::vector<map::TileCollisionData> result;
	if (map.getWidth() <= 0 || map.getHeight() <= 0 || radius <= 0.0f)
		return result;

	const CellCord minCord = map.worldToGrid(Vector3{pos.x - radius, 0.0f, pos.z - radius});
	const CellCord maxCord = map.worldToGrid(Vector3{pos.x + radius, 0.0f, pos.z + radius});

	const int startX = std::max(0, minCord.x);
	const int endX = std::min(map.getWidth() - 1, maxCord.x);
	const int startY = std::max(0, minCord.y);
	const int endY = std::min(map.getHeight() - 1, maxCord.y);

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
				result.push_back({tile, closestPoint});
			}
		}
	}

	return result;
}

std::optional<Vector3> map::MapCollider::calculateSphereCollisionNormals(
	const Map &map,
	Vector3 &pos,
	float radius
) {
	if (map.getWidth() <= 0 || map.getHeight() <= 0 || radius <= 0.0f)
		return std::nullopt;

	const std::vector<map::TileCollisionData> collidedTiles = collideTilesInRange(map, pos, radius);
	if (collidedTiles.empty())
		return std::nullopt;

	Vector3 totalNormal = {0.0f, 0.0f, 0.0f};

	for (const auto &[tile, contactPoint] : collidedTiles) {
		const Vector3 pointToSphere = pos - contactPoint;
		const float dist = Vector3Length(pointToSphere);
		const Vector3 normal = (dist > constants::epsilon) ? (pointToSphere / dist) : utils::math::getUpVector();
		const float penetration = radius - dist;
		pos += normal * penetration;
		totalNormal += normal;
	}
	if (Vector3LengthSqr(totalNormal) > constants::epsilon) {
		return Vector3Normalize(totalNormal);
	}

	return utils::math::getUpVector();
}
