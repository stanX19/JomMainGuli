#include "events.hpp"
#include "components.hpp"
#include "utils.hpp"
#include <algorithm>

using namespace component;

void event::Listener::handleCollectGuliEvent(const CollectGuliEvent &evt) {
	if (!evt.context)
		return;

	entt::registry &registry = evt.context->registry;
	if (!registry.valid(evt.player) || !registry.valid(evt.guli))
		return;

	if (!registry.all_of<tags::CollectibleGuli>(evt.guli))
		return;

	registry.remove<tags::CollectibleGuli>(evt.guli);
	::utils::entt::removeComponents<Velocity, CollisionBody, VerticalGravity, Mass, tags::RollsOnFloor, tags::zeroGravity>(registry, evt.guli);


	auto &inventory = registry.get_or_emplace<GuliInventory>(evt.player);
	inventory.guliCollection.push_back(evt.guli);

	const sound::Id soundId = evt.context->soundManager.getRandomGlassSound();
	if (soundId == sound::NONE)
		return;

	const auto *playerPos = registry.try_get<Position>(evt.player);
	const Vector3 pos = playerPos ? playerPos->value : Vector3{0.0f, 0.0f, 0.0f};

	evt.context->dispatcher.trigger<event::SoundEvent>(event::SoundEvent{
		evt.context,
		soundId,
		pos,
		1.0f
	});
}
