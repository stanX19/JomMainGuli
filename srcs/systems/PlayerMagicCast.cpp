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
	entt::entity spawnGraviton(GameContext &context, const Vector3 &pos, const Vector3 &vel = {0.0f, 0.0f, 0.0f}) {
		const entt::entity graviton = context.registry.create();
		context.registry.emplace<Position>(graviton, Position{pos});
		context.registry.emplace<Velocity>(graviton, Velocity{vel});
		return graviton;
	}

	void launchGravitonAndParticles(GameContext &context, entt::entity graviton, const Ray &ray) {
		if (graviton == entt::null || !context.registry.valid(graviton))
			return;

		const Vector3 targetWorldLoc = ray.position + ray.direction * 1000.0f;
		const float shootSpeed = context.config.magic.shootSpeed;

		auto [gravPos, gravVel] = context.registry.try_get<Position, Velocity>(graviton);
		if (!gravPos || !gravVel)
			return;

		const Vector3 shootDir = Vector3Normalize(targetWorldLoc - gravPos->value);
		gravVel->value = shootDir * shootSpeed;

		for (auto [entity, pos, vel, attracted] : context.registry.view<Position, Velocity, AttractedBy>().each()) {
			if (attracted.target != graviton)
				continue;

			vel.value += shootDir * shootSpeed;
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
	const Vector3 rayDir = Vector3Normalize(ray.direction);

	MagicCastState &castState = context.registry.get_or_emplace<MagicCastState>(context.currentPlayer, MagicCastState{
		.graviton = spawnGraviton(context, focusPoint, rayDir),
		.castTimer = 0.0f,
		.spawnTimer = 0.0f,
		.spawnCount = 0
	});

	auto [gravitonPos, gravitonVel] = context.registry.try_get<Position, Velocity>(castState.graviton);
	if (gravitonPos) {
		gravitonPos->value = focusPoint;
	}
	if (gravitonVel) {
		gravitonVel->value = rayDir;
	}

	castState.castTimer += dt;
	castState.spawnTimer += dt;

	if (castState.spawnCount < context.config.magic.maxSpawnCount && castState.spawnTimer >= context.config.magic.spawnInterval) {
		castState.spawnTimer = 0.0f;
		const std::optional<SpawnData> spawnData = sampleTileColor(context.map, playerPos, context.config.magic.spellCastRadius);
		if (spawnData) {
			entity::spawnMagicParticle(context, focusPoint, spawnData->color, castState.graviton, 0.2f);
			castState.spawnCount++;
		}
	}
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

	launchGravitonAndParticles(context, castState->graviton, ray);
	context.registry.remove<MagicCastState>(context.currentPlayer);
}
