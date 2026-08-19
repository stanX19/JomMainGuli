#include "entities.hpp"
#include "components.hpp"

using namespace component;

entt::entity entity::spawnMagicParticle(GameContext &context, Vector3 pos, Color color, entt::entity target, float radius) {
	const entt::entity particle = context.registry.create();

	context.registry.emplace<Position>(particle, Position{pos});
	context.registry.emplace<PrevPosition>(particle, PrevPosition{pos});
	context.registry.emplace<Velocity>(particle, Velocity{Vector3{0.0f, 0.0f, 0.0f}});
	context.registry.emplace<Rotation>(particle, Rotation{QuaternionIdentity()});
	context.registry.emplace<PrevRotation>(particle, PrevRotation{QuaternionIdentity()});
	context.registry.emplace<Mass>(particle, Mass{0.5f});
	context.registry.emplace<VerticalGravity>(particle, VerticalGravity{0.5f});

	const ModelId sphereModelId = context.modelManager.createSphere(16, 16, 1.0f);
	context.registry.emplace<RenderBody>(particle, RenderBody{sphereModelId, color, radius});
	context.registry.emplace<CollisionBody>(particle, CollisionBody{radius});

	context.registry.emplace<SpawnsTrailParticles>(particle, SpawnsTrailParticles{
		.radius = radius * 0.3f,
		.lifespan = 0.3f,
		.color = color,
	});
	if (target != entt::null) {
		context.registry.emplace<AttractedBy>(particle, AttractedBy{target, 100.0f});
	}

	return particle;
}
