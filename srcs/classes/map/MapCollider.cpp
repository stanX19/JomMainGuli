#include "constants.hpp"
#include "utils.hpp"
#include "map/MapCollider.hpp"
#include <algorithm>
#include <cmath>

std::vector<map::TileCollisionData> map::MapCollider::collideTilesInRange(
	const Map &map,
	const Vector3 &pos,
	const Vector3 &prevPos,
	float radius
) {
	std::vector<map::TileCollisionData> result;
	if (map.getWidth() <= 0 || map.getHeight() <= 0 || radius <= 0.0f)
		return result;

	const CellCord minCord = map.worldToGrid(Vector3{std::min(pos.x, prevPos.x) - radius, 0.0f, std::min(pos.z, prevPos.z) - radius});
	const CellCord maxCord = map.worldToGrid(Vector3{std::max(pos.x, prevPos.x) + radius, 0.0f, std::max(pos.z, prevPos.z) + radius});

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

std::vector<map::TileCollisionData> map::MapCollider::collideTilesInRange(
	const Map &map,
	Vector3 &pos,
	float radius
) {
	return collideTilesInRange(map, pos, pos, radius);
}

std::optional<Vector3> map::MapCollider::calculateSphereCollisionNormals(
	const Map &map,
	Vector3 &pos,
	const Vector3 &prevPos,
	float radius
) {
	if (map.getWidth() <= 0 || map.getHeight() <= 0 || radius <= 0.0f)
		return std::nullopt;

	const std::vector<map::TileCollisionData> collidedTiles = collideTilesInRange(map, pos, prevPos, radius);
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
		Vector3 groundNormal;
		const float groundY = map.getGroundY(pos, &groundNormal);

		const struct FaceCandidate {
			float dist;
			Vector3 normal;
			Vector3 newPos;
		} faces[5] = {
			{groundY - pos.y, groundNormal,                Vector3{pos.x, groundY + radius, pos.z}},
			{pos.x - tile.x1, Vector3{-1.0f, 0.0f,  0.0f}, Vector3{tile.x1 - radius, pos.y, pos.z}},
			{tile.x2 - pos.x, Vector3{ 1.0f, 0.0f,  0.0f}, Vector3{tile.x2 + radius, pos.y, pos.z}},
			{pos.z - tile.z1, Vector3{ 0.0f, 0.0f, -1.0f}, Vector3{pos.x, pos.y, tile.z1 - radius}},
			{tile.z2 - pos.z, Vector3{ 0.0f, 0.0f,  1.0f}, Vector3{pos.x, pos.y, tile.z2 + radius}},
		};

		const auto &closest = *std::min_element(
			std::begin(faces), std::end(faces),
			[](const FaceCandidate &a, const FaceCandidate &b) { return a.dist < b.dist; }
		);

		pos = closest.newPos;
		totalNormal += closest.normal;
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
