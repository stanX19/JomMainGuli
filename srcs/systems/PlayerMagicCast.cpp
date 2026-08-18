#include "systems/PlayerMagicCast.hpp"
#include "components.hpp"
#include "constants.hpp"
#include "entities.hpp"
#include "map/MapCollider.hpp"
#include "utils.hpp"
#include <algorithm>
#include <vector>

using namespace component;

namespace {
	constexpr float FOCUS_POINT_DISTANCE = 10.0f;
	constexpr float CLOSEST_TILE_SEARCH_RADIUS = 50.0f;
	constexpr float SHOOT_SPEED = 480.0f;
	constexpr float PARTICLE_SPAWN_INTERVAL = 0.15f;

	
	void updateHeldParticles(GameContext &context, const Vector3 &focusPoint, float dt) {
		for (auto [entity, pos, vel, casted] : context.registry.view<Position, Velocity, CastedParticle>().each()) {
			if (casted.caster != context.currentPlayer)
				continue;

			const Vector3 force = (focusPoint - pos.value) * 150.0f - vel.value * 8.0f;
			vel.value += force * dt;
			pos.value += vel.value * dt;
		}
	}

	void releaseHeldParticles(GameContext &context, const Ray &ray) {
		const Vector3 targetWorldLoc = ray.position + ray.direction * 1000.0f;
		std::vector<entt::entity> toRelease;

		for (auto [entity, casted] : context.registry.view<CastedParticle>().each()) {
			if (casted.caster == context.currentPlayer) {
				toRelease.push_back(entity);
			}
		}

		for (entt::entity entity : toRelease) {
			context.registry.remove<CastedParticle>(entity);

			auto [posPtr, velPtr] = context.registry.try_get<Position, Velocity>(entity);
			if (!posPtr || !velPtr)
				continue;

			const Vector3 shootDir = Vector3Normalize(targetWorldLoc - posPtr->value);
			velPtr->value += shootDir * SHOOT_SPEED;
		}
	}
}

std::optional<Color> systems::PlayerMagicCast::sampleTileColor(const map::Map &map, const Vector3 &samplePos, float searchRadius) {
	Vector3 queryPos = samplePos;
	const std::vector<map::TileCollisionData> tilesInRange
		= map::MapCollider::collideTilesInRange(map, queryPos, searchRadius);
	float distSum = 0.0f;
	std::vector<float> distWeights;
	distWeights.reserve(tilesInRange.size());

	for (const auto &[tile, contactPoint] : tilesInRange) {
		if (tile.color == map::ColorType::None) {
			distWeights.push_back(0.0f);
			continue;
		}
		const float dist = Vector3Distance(samplePos, contactPoint);
		const float weight = std::max(0.0f, 1.0f - (dist / searchRadius));
		distSum += weight;
		distWeights.push_back(weight);
	}

	if (distWeights.empty())
		return std::nullopt;

	const float chosenRng = m_uDist(m_rng) * distSum;
	float cumWeight = 0.0f;
	for (size_t i = 0; i < distWeights.size(); ++i) {
		cumWeight += distWeights[i];
		if (chosenRng < cumWeight) {
			return map::Map::getRaylibColor(tilesInRange[i].tile.color);
		}
	}

	return std::nullopt;
}

void systems::PlayerMagicCast::handleLMBDown(GameContext &context, const Ray &ray, const Vector3 &playerPos, float dt) {
	const Vector3 focusPoint = ray.position + ray.direction * FOCUS_POINT_DISTANCE;

	MagicCastState &castState = context.registry.get_or_emplace<MagicCastState>(context.currentPlayer, MagicCastState{
		.focusPoint = focusPoint,
		.castTimer = 0,
		.spawnTimer = 0
	});
	castState.focusPoint = focusPoint;
	castState.castTimer += dt;
	castState.spawnTimer += dt;

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || castState.spawnTimer >= PARTICLE_SPAWN_INTERVAL) {
		castState.spawnTimer = 0.0f;
		const std::optional<Color> particleColor = sampleTileColor(context.map, playerPos, CLOSEST_TILE_SEARCH_RADIUS);
		if (particleColor) {
			entity::spawnMagicParticle(context, focusPoint, *particleColor, context.currentPlayer);
		}
	}
	updateHeldParticles(context, focusPoint, dt);
}

void systems::PlayerMagicCast::update(GameContext &context, float dt) {
	if (!context.registry.valid(context.currentPlayer))
		return;

	auto [posPtr, rotPtr, unitCamera] = context.registry.try_get<Position, Rotation, UnitCamera>(context.currentPlayer);
	if (!posPtr || !rotPtr || !unitCamera)
		return;

	const Ray ray = GetScreenToWorldRay(GetMousePosition(), context.mainCamera);

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		handleLMBDown(context, ray, posPtr->value, dt);
		return;
	}

	MagicCastState *castState = context.registry.try_get<MagicCastState>(context.currentPlayer);
	if (!castState)
		return;
	context.registry.remove<MagicCastState>(context.currentPlayer);
	releaseHeldParticles(context, ray);
}
