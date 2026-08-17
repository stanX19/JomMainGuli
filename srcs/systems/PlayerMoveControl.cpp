#include "systems/PlayerMoveControl.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cmath>

using namespace component;

namespace {
	const Vector3 VectorXZ = {1, 0, 1};

	void processMouseLook(Rotation &rotation, const GameConfig &config) {
		const Vector2 mouseDelta = GetMouseDelta();
		if (mouseDelta.x == 0.0f && mouseDelta.y == 0.0f)
			return;

		const float sensitivity = 0.005f * config.settings.controlSensitivity;
		const Vector3 forward = utils::math::getForwardVector(rotation);

		const float currentYaw = std::atan2(forward.x, forward.z);
		const float currentPitch = std::asin(std::clamp(forward.y, -1.0f, 1.0f));

		const float maxPitch = 1.50f; // ~85.9 degrees
		const float newYaw = currentYaw - mouseDelta.x * sensitivity;
		const float newPitch = std::clamp(currentPitch - mouseDelta.y * sensitivity, -maxPitch, maxPitch);

		const Quaternion yawRot = QuaternionFromAxisAngle(utils::math::getUpVector(), newYaw);
		const Quaternion pitchRot = QuaternionFromAxisAngle(utils::math::getRightVector(), newPitch);

		rotation.value = QuaternionNormalize(QuaternionMultiply(yawRot, pitchRot));
	}

	void processMovement(Velocity &velocity, const Rotation &rotation, float dt) {
		const Vector3 frontVector = utils::math::getForwardVector(rotation);
		const Vector3 rightVector = utils::math::getRightVector(rotation);

		Vector3 forwardXZ = Vector3Normalize(frontVector * VectorXZ);
		Vector3 rightXZ = Vector3Normalize(rightVector * VectorXZ);

		Vector3 moveDir = {0.0f, 0.0f, 0.0f};
		if (IsKeyDown(KEY_W))
			moveDir += forwardXZ;
		if (IsKeyDown(KEY_S))
			moveDir -= forwardXZ;
		if (IsKeyDown(KEY_D))
			moveDir += rightXZ;
		if (IsKeyDown(KEY_A))
			moveDir -= rightXZ;

		const float maxSpeed = 40.0f;
		const float accel = 240.0f;

		Vector3 velocityChange;
		if (Vector3LengthSqr(moveDir) > 0.0001f) {
			moveDir = Vector3Normalize(moveDir);
			velocityChange = moveDir * accel * dt;
		} else {
			velocityChange = velocity.value * VectorXZ * -0.1;
		}

		velocity.value += velocityChange;

		const float XZSpeed = Vector2Length({velocity.value.x, velocity.value.z});
		if (XZSpeed > maxSpeed) {
			velocity.value -= velocity.value * VectorXZ
			            	* (XZSpeed - maxSpeed) / XZSpeed;
		}

		if (IsKeyDown(KEY_SPACE)) {
			const float upAccel = accel;
			velocity.value.y += upAccel * dt;
		}
	}
}

void systems::PlayerMoveControl::update(GameContext &context, float dt) {
	if (!context.registry.valid(context.currentPlayer))
		return;

	auto [rotPtr, velPtr] = context.registry.try_get<Rotation, Velocity>(context.currentPlayer);
	if (!rotPtr || !velPtr)
		return;

	processMouseLook(*rotPtr, context.config);
	processMovement(*velPtr, *rotPtr, dt);
}
