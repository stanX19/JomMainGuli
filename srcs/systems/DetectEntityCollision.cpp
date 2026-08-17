#include "systems/DetectEntityCollision.hpp"
#include "utils.hpp"
#include "events.hpp"
#include <algorithm>
#include <vector>

using namespace component;

namespace {
	struct SphereEntityData {
		entt::entity id;
		Vector3 pos;
		Vector3 vel;
		float radius;
	};
}

void systems::DetectEntityCollision::update(GameContext &context, float dt) {
	std::vector<SphereEntityData> entities;

	for (auto [entity, position, colBody] : context.registry.view<Position, CollisionBody>().each()) {
		Vector3 velocity = {0.0f, 0.0f, 0.0f};
		if (PrevPosition *prev = context.registry.try_get<PrevPosition>(entity)) {
			velocity = position.value - prev->value;
		}

		entities.emplace_back(SphereEntityData{
			entity,
			position.value - velocity,
			velocity,
			colBody.radius
		});
	}

	const std::size_t count = entities.size();
	for (std::size_t i = 0; i < count; ++i) {
		const SphereEntityData &A = entities[i];

		for (std::size_t j = i + 1; j < count; ++j) {
			const SphereEntityData &B = entities[j];

			const float combinedRadius = A.radius + B.radius;
			std::optional<utils::collision::CollisionInterval> interval = utils::collision::calculateCollisionInterval(
				A.pos,
				A.vel,
				B.pos,
				B.vel,
				combinedRadius
			);

			if (!utils::collision::willCollide(interval, 1.0f))
				continue;

			const float collisionDt = std::max(interval->collisionStartDt, 0.0f);
			context.dispatcher.enqueue<event::CollisionEvent>(event::CollisionEvent{
				&context,
				event::CollisionParty{A.id, A.pos + A.vel * collisionDt, A.vel / dt},
				event::CollisionParty{B.id, B.pos + B.vel * collisionDt, B.vel / dt},
				dt,
				collisionDt
			});
		}
	}
}
