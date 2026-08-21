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
			const float clampedZ = std::clamp(pos.z, tile.z1, tile.z2);

			Vector3 normal;
			const float groundY = map.getGroundY(Vector3{clampedX, pos.y, clampedZ}, &normal);
			const Vector3 closestPoint = {
				clampedX,
				std::clamp(pos.y, -100.0f, groundY),
				clampedZ
			};

			if (Vector3LengthSqr(pos - closestPoint) < radius * radius) {
				result.push_back({tile, closestPoint});
			}
		}
	}

	return result;
}

std::optional<Vector3> map::MapCollider::calculateSphereCollisionNormals(
	const Map &map,
	Vector3 &pos,
	const Vector3 &prevPos,
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

		if (dist > constants::epsilon) {
			const Vector3 normal = pointToSphere / dist;
			pos += normal * (radius - dist);
			totalNormal += normal;
			continue;
		}
		// else, its inside the cell body already
		// use prev pos to find out entering face
		Vector3 groundNormal;
		const float groundY = map.getGroundY(pos, &groundNormal);

		if (prevPos.y >= groundY) {
			pos.y = groundY + radius;
			totalNormal += groundNormal;
		} else if (prevPos.x <= tile.x1) {
			pos.x = tile.x1 - radius;
			totalNormal += Vector3{-1.0f, 0.0f, 0.0f};
		} else if (prevPos.x >= tile.x2) {
			pos.x = tile.x2 + radius;
			totalNormal += Vector3{1.0f, 0.0f, 0.0f};
		} else if (prevPos.z <= tile.z1) {
			pos.z = tile.z1 - radius;
			totalNormal += Vector3{0.0f, 0.0f, -1.0f};
		} else if (prevPos.z >= tile.z2) {
			pos.z = tile.z2 + radius;
			totalNormal += Vector3{0.0f, 0.0f, 1.0f};
		}
	}

	if (Vector3LengthSqr(totalNormal) > constants::epsilon)
		return Vector3Normalize(totalNormal);

	return utils::math::getUpVector();
}

std::optional<Vector3> map::MapCollider::calculateSphereCollisionNormals(
	const Map &map,
	Vector3 &pos,
	float radius
) {
	return calculateSphereCollisionNormals(map, pos, pos, radius);
}
