#include "events.hpp"
#include "components.hpp"
#include <algorithm>

using namespace component;

namespace {
	void applyCollisionPhysics(const event::CollisionEvent &evt) {
		const auto& a = evt.a;
		const auto& b = evt.b;
		
		if (!evt.context)
			return;
		entt::registry &registry = evt.context->registry;
		
		auto [aMass, aVel, aRot] = registry.try_get<Mass, Velocity, Rotation>(a.id);
		auto [bMass, bVel, bRot] = registry.try_get<Mass, Velocity, Rotation>(b.id);

		if (!aMass || !aVel || !bMass || !bVel)
			return;

		if (aMass->value <= 0.0f || bMass->value <= 0.0f)
			return;

		const Vector3 normal = Vector3Normalize(b.pos - a.pos);
		const Vector3 relativeVelocity = b.vel - a.vel;
		const float velAlongNormal = Vector3DotProduct(relativeVelocity, normal);

		if (velAlongNormal >= 0.0f)
			return;

		const float collisionElasticity = evt.context->config.physics.collisionElasticity;
		const float invMassA = 1.0f / aMass->value;
		const float invMassB = 1.0f / bMass->value;
		const float impulseScalar = (-(1.0f + collisionElasticity) * velAlongNormal) / (invMassA + invMassB);
		const Vector3 impulse = normal * impulseScalar;

		aVel->value -= impulse * invMassA;
		bVel->value += impulse * invMassB;

		if (!aRot || !bRot)
			return;

		const Vector3 torqueAxis = Vector3CrossProduct(normal, relativeVelocity);
		const float torqueMagnitude = Vector3Length(torqueAxis);

		if (torqueMagnitude <= 0.1f)
			return;

		const float roughness = evt.context->config.physics.roughness;
		const float maxKick = evt.context->config.physics.maxAngularKick;

		const float aKick = std::min((bMass->value / aMass->value) * torqueMagnitude * roughness, maxKick);
		const Quaternion aSpin = QuaternionFromAxisAngle(Vector3Normalize(torqueAxis), aKick);
		aRot->value = QuaternionMultiply(aSpin, aRot->value);

		const float bKick = std::min((aMass->value / bMass->value) * torqueMagnitude * roughness, maxKick);
		const Quaternion bSpin = QuaternionFromAxisAngle(Vector3Normalize(torqueAxis), -bKick);
		bRot->value = QuaternionMultiply(bRot->value, bSpin);
	}

	void triggerCollisionSound(const event::CollisionEvent &evt) {
		if (!evt.context)
			return;

		const auto &registry = evt.context->registry;
		const bool aIsGlass = registry.valid(evt.a.id) && registry.all_of<tags::GlassCollisionSound>(evt.a.id);
		const bool bIsGlass = registry.valid(evt.b.id) && registry.all_of<tags::GlassCollisionSound>(evt.b.id);

		if (!aIsGlass && !bIsGlass)
			return;

		const Vector3 normal = Vector3Normalize(evt.b.pos - evt.a.pos);
		const Vector3 relativeVelocity = evt.b.vel - evt.a.vel;
		const float velAlongNormal = Vector3DotProduct(relativeVelocity, normal);

		if (velAlongNormal >= -1.5f)
			return;

		const float impactSpeed = -velAlongNormal;
		const sound::Id soundId = evt.context->soundManager.getRandomGlassSound();
		if (soundId == sound::NONE)
			return;

		const Vector3 collisionPos = (evt.a.pos + evt.b.pos) * 0.5f;
		const float volume = std::clamp(impactSpeed / 120.0f, 0.15f, 1.0f);

		evt.context->dispatcher.trigger<event::SoundEvent>(event::SoundEvent{
			evt.context,
			soundId,
			collisionPos,
			volume
		});
	}
	void checkCollectGuli(const event::CollisionEvent &evt) {
		if (!evt.context)
			return;

		const auto &registry = evt.context->registry;
		if (!registry.valid(evt.context->currentPlayer))
			return;

		const entt::entity player = evt.context->currentPlayer;
		entt::entity guli = entt::null;

		if (evt.a.id == player && registry.valid(evt.b.id) && registry.all_of<tags::CollectibleGuli>(evt.b.id)) {
			guli = evt.b.id;
		} else if (evt.b.id == player && registry.valid(evt.a.id) && registry.all_of<tags::CollectibleGuli>(evt.a.id)) {
			guli = evt.a.id;
		}

		if (guli == entt::null)
			return;

		evt.context->dispatcher.enqueue<event::CollectGuliEvent>(event::CollectGuliEvent{
			evt.context,
			player,
			guli
		});
	}

	void checkCheatCollisionToast(const event::CollisionEvent &evt) {
		if (!evt.context)
			return;
		if (evt.context->state.cheatToastCooldown > 0.0f)
			return;

		const auto &registry = evt.context->registry;
		if (!registry.valid(evt.context->currentPlayer))
			return;

		const entt::entity player = evt.context->currentPlayer;
		const bool isFoul =
			(evt.a.id == player && registry.valid(evt.b.id) && registry.all_of<tags::GuliTarget>(evt.b.id)) ||
			(evt.b.id == player && registry.valid(evt.a.id) && registry.all_of<tags::GuliTarget>(evt.a.id));

		if (!isFoul)
			return;

		static const char *const foulMessages[] = {"CHEAT", "FOUL", "HANDS OFF"};
		const int idx = GetRandomValue(0, 2);

		evt.context->state.addToast(foulMessages[idx], ORANGE, 1.0f, 36, ToastPriority::LOW);
		evt.context->state.cheatToastCooldown = 1.0f;
	}

	void checkStrikeCollisionToast(const event::CollisionEvent &evt) {
		if (!evt.context)
			return;
		if (evt.context->state.canShowStrikeToast <= 0.0f)
			return;

		const auto &registry = evt.context->registry;
		const bool isStrike =
			(registry.valid(evt.a.id) && registry.all_of<ShotGuli>(evt.a.id) &&
			 registry.valid(evt.b.id) && registry.all_of<tags::GuliTarget>(evt.b.id)) ||
			(registry.valid(evt.b.id) && registry.all_of<ShotGuli>(evt.b.id) &&
			 registry.valid(evt.a.id) && registry.all_of<tags::GuliTarget>(evt.a.id));

		if (!isStrike)
			return;

		static const char *const strikeMessages[] = {"STRIKE", "HIT", "CONTACT", "IMPACT"};
		const int idx = GetRandomValue(0, 3);

		evt.context->state.addToast(strikeMessages[idx], GOLD, 1.0f, 36, ToastPriority::LOW);
		evt.context->state.canShowStrikeToast = 0.0f;
	}
}

void event::Listener::handleCollisionEvent(const CollisionEvent &evt) {
	checkCollectGuli(evt);
	checkCheatCollisionToast(evt);
	checkStrikeCollisionToast(evt);
	applyCollisionPhysics(evt);
	triggerCollisionSound(evt);
}


