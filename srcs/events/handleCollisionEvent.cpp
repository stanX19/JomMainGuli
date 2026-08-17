#include "events.hpp"
#include "components.hpp"
#include <algorithm>

using namespace component;

namespace {
	void applyCollisionPhysics(const event::CollisionEvent &evt) {
		const auto& a = evt.a;
		const auto& b = evt.b;
		if (!evt.context) return;
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
}

void event::Listener::handleCollisionEvent(const CollisionEvent &evt) {
	applyCollisionPhysics(evt);
}
