#pragma once

#include "includes.hpp"
#include "GameContext.hpp"
#include "SoundManager.hpp"

namespace event {
	struct CollisionParty {
		entt::entity id = entt::null;
		Vector3 pos = {0.0f, 0.0f, 0.0f};
		Vector3 vel = {0.0f, 0.0f, 0.0f};
	};

	struct CollisionEvent {
		GameContext *context = nullptr;
		CollisionParty a;
		CollisionParty b;
		float dt = 0.0f;
		float collisionDtRatio = 0.0f;
	};

	struct SoundEvent {
		GameContext *context = nullptr;
		sound::Id soundId = sound::NONE;
		Vector3 position = {0.0f, 0.0f, 0.0f};
		float volume = 1.0f;
	};

	struct Listener {
		void handleCollisionEvent(const CollisionEvent& evt);
		void handleSoundEvent(const SoundEvent& evt);
	};
}

namespace event::utils {
	template<typename Event, auto MemberFunc>
	void hookToDispatcher(GameContext& context) {
		static event::Listener listener;
		context.dispatcher.sink<Event>().template connect<MemberFunc>(listener);
	}

	void hookAllListeners(GameContext& context);
}