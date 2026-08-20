#include "systems/EntityWorldCollision.hpp"
#include "map/Map.hpp"
#include "map/MapCollider.hpp"
#include "utils.hpp"
#include "events.hpp"
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
}

void systems::EntityWorldCollision::update(GameContext &context, [[maybe_unused]] float dt) {
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

		const float e = (velAlongNormal > -2.0f) ? 0.0f : elasticity;
		const Vector3 normalVel = *normal * velAlongNormal;
		const Vector3 tangentVel = vel.value - normalVel;
		vel.value = normalVel * -e + tangentVel * 0.95f;

		spawnCollisionSoundIfEligible(context, entity, pos.value, velAlongNormal);
	}
}
