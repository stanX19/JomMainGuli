#include "systems/EntityMovement.hpp"
#include "utils.hpp"

using namespace component;

void systems::EntityMovement::update(GameContext &context, float dt) {
	for (auto [entity, velocity, scalarAcceleration] : context.registry.view<Velocity, ScalarAcceleration>().each()) {
		if (Vector3Length(velocity.value) > 0.001f) {
			velocity.value += Vector3Normalize(velocity.value) * (scalarAcceleration.value * dt);
		}
	}

	for (auto [entity, position, velocity] : context.registry.view<Position, Velocity>().each()) {
		context.registry.emplace_or_replace<PrevPosition>(entity, position.value);
		position.value = position.value + velocity.value * dt;
	}

	for (auto [entity, rotation, rotVel] : context.registry.view<Rotation, RotationVelocity>().each()) {
		Quaternion delta = QuaternionLerp(QuaternionIdentity(), rotVel.value, dt);
		context.registry.emplace_or_replace<PrevRotation>(entity, rotation.value);
		rotation.value = QuaternionMultiply(rotation.value, delta);
	}
}
