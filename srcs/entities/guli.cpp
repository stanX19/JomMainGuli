#include "entities.hpp"
#include "components.hpp"

using namespace component;

entt::entity entity::spawnGuli(GameContext &context, Vector3 pos, float radius) {
	const entt::entity guli = context.registry.create();

	context.registry.emplace<Position>(guli, Position{pos});
	context.registry.emplace<PrevPosition>(guli, PrevPosition{pos});
	context.registry.emplace<Velocity>(guli, Velocity{Vector3{0.0f, 0.0f, 0.0f}});
	context.registry.emplace<Rotation>(guli, Rotation{QuaternionIdentity()});
	context.registry.emplace<PrevRotation>(guli, PrevRotation{QuaternionIdentity()});
	context.registry.emplace<Mass>(guli, Mass{4.0f});
	context.registry.emplace<VerticalGravity>(guli, VerticalGravity{1.0f});

	const ModelId sphereModelId = context.modelManager.createSphere(16, 16, 1.0f);
	context.registry.emplace<RenderBody>(guli, RenderBody{sphereModelId, WHITE, radius});
	context.registry.emplace<CollisionBody>(guli, CollisionBody{radius});
	context.registry.emplace<tags::GuliTarget>(guli);

	return guli;
}
