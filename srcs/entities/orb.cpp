#include "entities.hpp"
#include "components.hpp"

using namespace component;

entt::entity entity::spawnOrb(GameContext &context, Vector3 pos, Color color, float radius) {
	const entt::entity orb = context.registry.create();

	context.registry.emplace<Position>(orb, Position{pos});
	context.registry.emplace<PrevPosition>(orb, PrevPosition{pos});
	context.registry.emplace<Velocity>(orb, Velocity{Vector3{0.0f, 0.0f, 0.0f}});
	context.registry.emplace<Rotation>(orb, Rotation{QuaternionIdentity()});
	context.registry.emplace<PrevRotation>(orb, PrevRotation{QuaternionIdentity()});
	context.registry.emplace<Mass>(orb, Mass{5.0f});

	const ModelId sphereModelId = context.modelManager.createSphere(16, 16, 1.0f);
	context.registry.emplace<RenderBody>(orb, RenderBody{sphereModelId, color, radius});
	context.registry.emplace<CollisionBody>(orb, CollisionBody{radius});
	context.registry.emplace<tags::GlassCollisionSound>(orb);

	return orb;
}
