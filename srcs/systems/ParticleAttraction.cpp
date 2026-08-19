#include "systems/ParticleAttraction.hpp"
#include "components.hpp"
#include "constants.hpp"
#include "utils.hpp"

using namespace component;

void systems::ParticleAttraction::update(GameContext &context, float dt) {
	for (auto [entity, pos, vel, attracted] : context.registry.view<Position, Velocity, AttractedBy>().each()) {
		context.registry.remove<tags::zeroGravity>(entity);

		if (attracted.target == entt::null || !context.registry.valid(attracted.target)) {
			context.registry.remove<AttractedBy>(entity);
			continue;
		}

		auto [gravitonPos, gravitonVel] = context.registry.try_get<Position, Velocity>(attracted.target);
		if (!gravitonPos || !gravitonVel)
			continue;

		const float castRadius = context.config.magic.spellCastRadius;
		const Vector3 velChange = utils::algorithm::calculateVortexAttractionVelocity(
			pos.value,
			gravitonPos->value,
			gravitonVel->value,
			attracted.strength,
			castRadius,
			0.01f
		);

		vel.value += velChange * dt;
		const Vector3 relVel = vel.value - gravitonVel->value;
		vel.value = gravitonVel->value + relVel * 0.99f;

		// anti gravity effect
		context.registry.emplace_or_replace<tags::zeroGravity>(entity);
	}
}
