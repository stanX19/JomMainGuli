#include "systems/EntityMovement.hpp"
#include "utils.hpp"
#include "constants.hpp"

using namespace component;

namespace {
	void processGravity(GameContext &context, float dt) {
		for (auto [entity, vel, gravity] : context.registry.view<component::Velocity, component::VerticalGravity>(entt::exclude<tags::zeroGravity>).each()) {
			vel.value += utils::math::getUpVector() * context.config.physics.gravity * -gravity.value * dt;
		}
	}

	void processScalarAcceleration(GameContext &context, float dt) {
		for (auto [entity, velocity, scalarAcceleration] : context.registry.view<Velocity, ScalarAcceleration>().each()) {
			if (Vector3Length(velocity.value) > constants::epsilon) {
				velocity.value += Vector3Normalize(velocity.value) * (scalarAcceleration.value * dt);
			}
		}
	}

	void processPosTranslation(GameContext &context, float dt) {
		for (auto [entity, position, velocity] : context.registry.view<Position, Velocity>().each()) {
			context.registry.emplace_or_replace<PrevPosition>(entity, position.value);
			position.value = position.value + velocity.value * dt;
		}
	}

	void processRotationalVelocity(GameContext &context, float dt) {
		for (auto [entity, rotation, rotVel] : context.registry.view<Rotation, RotationVelocity>().each()) {
			Quaternion delta = QuaternionLerp(QuaternionIdentity(), rotVel.value, dt);
			context.registry.emplace_or_replace<PrevRotation>(entity, rotation.value);
			rotation.value = QuaternionMultiply(rotation.value, delta);
		}
	}
}

void systems::EntityMovement::update(GameContext &context, float dt) {
	processGravity(context, dt);
	processScalarAcceleration(context, dt);
	processRotationalVelocity(context, dt);
	processPosTranslation(context, dt);
}
