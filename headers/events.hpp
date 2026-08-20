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

	struct CollectGuliEvent {
		GameContext *context = nullptr;
		entt::entity player = entt::null;
		entt::entity guli = entt::null;
	};

	struct Listener {
		void handleCollisionEvent(const CollisionEvent& evt);
		void handleSoundEvent(const SoundEvent& evt);
		void handleCollectGuliEvent(const CollectGuliEvent& evt);
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