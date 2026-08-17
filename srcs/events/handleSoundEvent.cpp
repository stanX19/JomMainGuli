#include "events.hpp"

void event::Listener::handleSoundEvent(const SoundEvent &evt) {
	if (!evt.context || evt.soundId == sound::NONE) return;
	evt.context->soundManager.queueSound(evt.soundId, evt.position, evt.volume);
}
