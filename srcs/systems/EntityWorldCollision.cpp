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
		std::optional<Vector3> normal = map::MapCollider::calculateSphereCollisionNormals(map, pos.value, body.radius);
		if (!normal.has_value())
			continue;

		const float velAlongNormal = Vector3DotProduct(vel.value, *normal);
		if (velAlongNormal < 0.0f) {
			const float e = (std::abs(velAlongNormal) < 2.0f) ? 0.0f : elasticity;
			vel.value -= *normal * ((1.0f + e) * velAlongNormal);
		}
	}
}
