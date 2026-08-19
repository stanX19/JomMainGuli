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

		const Vector3 toTarget = gravitonPos->value - pos.value;
		const float distSq = Vector3LengthSqr(toTarget);
		const float castRadius = context.config.magic.spellCastRadius;

		if (distSq > castRadius * castRadius || distSq < constants::epsilon)
			continue;

		const Vector3 toTargetNorm = Vector3Normalize(toTarget);
		const Vector3 gravitonVelNorm = Vector3Normalize(gravitonVel->value);
		const Vector3 displacedDir = Vector3CrossProduct(gravitonVelNorm, toTargetNorm); // for swirl
		const float strength = attracted.strength * Clamp(1.0f - (distSq / (castRadius * castRadius)), 0.1f, 1.0f);
		const Vector3 velChange = (toTargetNorm + displacedDir * 0.01f) * strength;

		vel.value += velChange * dt;
		const Vector3 relVel = vel.value - gravitonVel->value;
		vel.value = gravitonVel->value + relVel * 0.99f;

		// anti gravity effect
		context.registry.emplace_or_replace<tags::zeroGravity>(entity);
	}
}
