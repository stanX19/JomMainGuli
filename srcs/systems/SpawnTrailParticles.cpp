#include "systems/SpawnTrailParticles.hpp"
#include "entities.hpp"

using namespace component;

void systems::SpawnTrailParticles::update(GameContext &context, [[maybe_unused]] float dt) {
	for (auto [entity, trail, pos, vel] : context.registry.view<SpawnsTrailParticles, Position, Velocity>().each()) {
		if (Vector3Length(vel.value) < 10.0f)
			continue;

		const entt::entity particle = context.registry.create();
		context.registry.emplace<Position>(particle, Position{pos.value});
		context.registry.emplace<Velocity>(particle, Velocity{vel.value * 0.5f});
		context.registry.emplace<RenderBody>(
			particle,
			context.modelManager.createSphere(6, 6),
			ColorAlpha(trail.color, 0.5f),
			trail.radius
		);
		context.registry.emplace<RadiusExpand>(
			particle,
			-trail.radius / trail.lifespan * 0.75f
		);
		context.registry.emplace<Lifespan>(particle, trail.lifespan);
	}
}
