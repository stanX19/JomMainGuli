#include "entities.hpp"
#include "components.hpp"
#include "utils.hpp"

using namespace component;

entt::entity entity::spawnPlayer(GameContext &context) {
	return entity::spawnPlayer(context, Vector3{0.0f, 0.0f, 0.0f});
}

entt::entity entity::spawnPlayer(GameContext &context, Vector3 pos) {
	const entt::entity player = context.registry.create();

	context.registry.emplace<Position>(player, Position{pos});
	context.registry.emplace<PrevPosition>(player, PrevPosition{pos});
	context.registry.emplace<Velocity>(player, Velocity{Vector3{0.0f, 0.0f, 0.0f}});
	context.registry.emplace<Rotation>(player, Rotation{QuaternionIdentity()});
	context.registry.emplace<PrevRotation>(player, PrevRotation{QuaternionIdentity()});
	context.registry.emplace<Mass>(player, Mass{10.0f});

	const ModelId sphereModelId = context.modelManager.createSphere(16, 16, 1.0f);
	context.registry.emplace<RenderBody>(player, RenderBody{sphereModelId, SKYBLUE, 2.0f});
	context.registry.emplace<CollisionBody>(player, CollisionBody{2.0f});

	SpawnsTrailParticles trail{};
	trail.spawnCount = 1;
	trail.spawnLocations[0] = Vector3{0.0f, 0.0f, -1.0f};
	trail.radius = 0.6f;
	trail.lifespan = 0.8f;
	trail.color = SKYBLUE;
	context.registry.emplace<SpawnsTrailParticles>(player, trail);

	context.registry.emplace<UnitCamera>(player);

	context.currentPlayer = player;
	return player;
}
