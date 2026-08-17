#include "events.hpp"

void event::utils::hookAllListeners(GameContext& context) {
	hookToDispatcher<event::CollisionEvent, &event::Listener::handleCollisionEvent>(context);
	hookToDispatcher<event::SoundEvent, &event::Listener::handleSoundEvent>(context);
}