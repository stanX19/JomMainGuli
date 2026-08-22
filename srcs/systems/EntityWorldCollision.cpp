#include "systems/EntityWorldCollision.hpp"
#include "map/Map.hpp"
#include "map/MapCollider.hpp"
#include "utils.hpp"
#include "events.hpp"
#include "constants.hpp"
#include <algorithm>
#include <cmath>

using namespace component;

namespace {
	void spawnCollisionSoundIfEligible(GameContext &context, entt::entity entity, const Vector3 &position, float velAlongNormal) {
		if (velAlongNormal >= -5.0f || !context.registry.all_of<tags::GlassCollisionSound>(entity))
			return;

		const sound::Id sndId = context.soundManager.getRandomGlassSound();
		if (sndId == sound::NONE)
			return;

		const float impactSpeed = -velAlongNormal;
		const float volume = std::clamp(impactSpeed / 60.0f, 0.1f, 0.8f);
		context.dispatcher.trigger<event::SoundEvent>(event::SoundEvent{
			&context,
			sndId,
			position,
			volume
		});
	}

	void applyRollingRotation(GameContext &context, entt::entity entity, const Vector3 &normal, const Vector3 &tangentVel, float radius, float dt) {
		if (radius <= constants::epsilon || dt <= 0.0f || !context.registry.all_of<tags::RollsOnFloor>(entity))
			return;

		auto *rot = context.registry.try_get<Rotation>(entity);
		if (!rot)
			return;

		const float speed = Vector3Length(tangentVel);
		if (speed <= constants::epsilon)
			return;

		const Vector3 rollAxis = Vector3Normalize(Vector3CrossProduct(normal, tangentVel));
		const float deltaAngle = (speed / radius) * dt;
		const Quaternion deltaRot = QuaternionFromAxisAngle(rollAxis, deltaAngle);
		rot->value = QuaternionNormalize(QuaternionMultiply(deltaRot, rot->value));
	}
}

void systems::EntityWorldCollision::update(GameContext &context, float dt) {
	const map::Map &map = context.map;
	const float elasticity = context.config.physics.collisionElasticity;

	for (auto [entity, pos, vel, body] : context.registry.view<Position, Velocity, CollisionBody>().each()) {
		Vector3 prevPos = pos.value;
		if (const PrevPosition *prev = context.registry.try_get<PrevPosition>(entity)) {
			prevPos = prev->value;
		}

		std::optional<Vector3> normal = map::MapCollider::calculateSphereCollisionNormals(map, pos.value, prevPos, body.radius);
		if (!normal.has_value())
			continue;

		const float velAlongNormal = Vector3DotProduct(vel.value, *normal);
		if (velAlongNormal >= 0.0f)
			continue;
		
		const float e = (velAlongNormal > -20.0f) ? 0.0f : elasticity;
		const Vector3 normalVel = *normal * velAlongNormal;
		const Vector3 tangentVel = vel.value - normalVel;
		vel.value = normalVel * -e + tangentVel * 0.99f;
		applyRollingRotation(context, entity, *normal, tangentVel, body.radius, dt);

		spawnCollisionSoundIfEligible(context, entity, pos.value, velAlongNormal);
	}
}

