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
	void updateHeldParticles(GameContext &context, const Vector3 &focusPoint, const Vector3 &rayDirection, float dt, float castRadius) {
		for (auto [entity, pos, vel, casted] : context.registry.view<Position, Velocity, CastedParticle>().each()) {
			if (casted.caster != context.currentPlayer)
				continue;

			const Vector3 toFocus = focusPoint - pos.value;
			const float distSq = Vector3LengthSqr(toFocus);

			if (distSq > castRadius * castRadius) {
				continue;
			}
			const Vector3 toFocusNorm = Vector3Normalize(toFocus);
			const Vector3 displacedDir = Vector3CrossProduct(rayDirection, toFocusNorm);
			const float strength = 100.0f * Clamp(1.0f - (distSq / (castRadius * castRadius)), 0.1f, 1.0f);
			const Vector3 velChange = (toFocusNorm + displacedDir * 0.01f) * strength;
			vel.value += velChange * dt;
			vel.value *= 0.99f; // damping
		}
	}

	void releaseHeldParticles(GameContext &context, const Ray &ray, const Vector3 &focusPoint) {
		const Vector3 targetWorldLoc = ray.position + ray.direction * 1000.0f;
		std::vector<entt::entity> toRelease;

		for (auto [entity, casted] : context.registry.view<CastedParticle>().each()) {
			if (casted.caster == context.currentPlayer) {
				toRelease.push_back(entity);
			}
		}

		const float shootSpeed = context.config.magic.shootSpeed;
		for (entt::entity entity : toRelease) {
			context.registry.remove<CastedParticle>(entity);

			auto [posPtr, velPtr] = context.registry.try_get<Position, Velocity>(entity);
			if (!posPtr || !velPtr)
				continue;

			if (Vector3Distance(posPtr->value, focusPoint) > context.config.magic.spellCastRadius) {
				continue;
			}

			const Vector3 shootDir = Vector3Normalize(targetWorldLoc - posPtr->value);
			velPtr->value += shootDir * shootSpeed;
		}
	}
}
	
std::optional<systems::PlayerMagicCast::SpawnData> systems::PlayerMagicCast::sampleTileColor(const map::Map &map, const Vector3 &samplePos, float searchRadius) {
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
			return SpawnData{
				.color = map::Map::getRaylibColor(tilesInRange[i].tile.color),
				.loc = tilesInRange[i].contactPoint
			};
		}
	}

	return std::nullopt;
}

void systems::PlayerMagicCast::handleLMBDown(GameContext &context, const Ray &ray, const Vector3 &playerPos, float dt) {
	const Vector3 focusPoint = ray.position + ray.direction * context.config.magic.focusDistance;

	MagicCastState &castState = context.registry.get_or_emplace<MagicCastState>(context.currentPlayer, MagicCastState{
		.focusPoint = focusPoint,
		.castTimer = 0,
		.spawnTimer = 0,
		.spawnCount = 0
	});
	castState.focusPoint = focusPoint;
	castState.castTimer += dt;
	castState.spawnTimer += dt;

	if (castState.spawnCount < context.config.magic.maxSpawnCount && castState.spawnTimer >= context.config.magic.spawnInterval) {
		castState.spawnTimer = 0.0f;
		const std::optional<SpawnData> spawnData = sampleTileColor(context.map, playerPos, context.config.magic.spellCastRadius);
		if (spawnData) {
			entity::spawnMagicParticle(context, focusPoint, spawnData->color, context.currentPlayer, 0.2f);
			castState.spawnCount++;
		}
	}
	const Vector3 rayDir = Vector3Normalize(ray.direction);
	updateHeldParticles(context, focusPoint, rayDir, dt, context.config.magic.spellCastRadius);
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
	releaseHeldParticles(context, ray, castState->focusPoint);
}
