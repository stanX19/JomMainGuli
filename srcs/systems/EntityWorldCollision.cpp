#include "systems/EntityWorldCollision.hpp"
#include "map/Map.hpp"
#include "map/MapCollider.hpp"
#include "utils.hpp"
#include <cmath>

using namespace component;

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
		if (velAlongNormal < 0.0f) {
			const float e = (std::abs(velAlongNormal) < 2.0f) ? 0.0f : elasticity;
			const Vector3 normalVel = *normal * velAlongNormal;
			const Vector3 tangentVel = vel.value - normalVel;
			vel.value = normalVel * -e + tangentVel * 0.99f;
		}
	}
}
