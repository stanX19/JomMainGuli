#include "systems/PlayerMoveControl.hpp"
#include "utils.hpp"
#include <cmath>

using namespace component;

namespace {
	Vector2 getMouseDirectionRelRot(
		const Quaternion &entRot,
		const Camera3D &camera,
		float sensitivity
	) {
		const float mouseUnitRatio = 0.8f;
		const Vector2 mouseDirection = utils::input::getMouseDirectionNormalized(mouseUnitRatio) * sensitivity;

		if (std::abs(mouseDirection.x) < 0.01f && std::abs(mouseDirection.y) < 0.01f) {
			return {0.0f, 0.0f};
		}

		const Vector3 entityUp = utils::math::getUpVector(entRot);
		const Vector3 entityRight = utils::math::getRightVector(entRot);

		const Vector3 cameraForward = Vector3Normalize(camera.target - camera.position);
		const Vector3 cameraRight = Vector3Normalize(Vector3CrossProduct(cameraForward, camera.up));

		const Vector2 entityFlatUp = Vector2Normalize(Vector2{
			Vector3DotProduct(entityUp, cameraRight),
			-Vector3DotProduct(entityUp, camera.up)
		});

		const Vector2 entityFlatRight = Vector2Normalize(Vector2{
			Vector3DotProduct(entityRight, cameraRight),
			-Vector3DotProduct(entityRight, camera.up)
		});

		return Vector2{
			-Vector2DotProduct(entityFlatRight, mouseDirection),
			-Vector2DotProduct(entityFlatUp, mouseDirection)
		};
	}
}

void systems::PlayerMoveControl::update(GameContext &context, float dt) {
	if (!context.registry.valid(context.currentPlayer))
		return;

	auto [posPtr, rotPtr, velPtr] = context.registry.try_get<Position, Rotation, Velocity>(context.currentPlayer);
	if (!posPtr || !rotPtr || !velPtr)
		return;

	Position &position = *posPtr;
	Rotation &rotation = *rotPtr;
	Velocity &velocity = *velPtr;

	const float turnSpeedDt = 2.5f * dt;
	Quaternion newRotation = rotation.value;
	const Vector3 upVector = utils::math::getUpVector(rotation);
	const Vector3 rightVector = utils::math::getRightVector(rotation);

	const Vector2 mouseDir = getMouseDirectionRelRot(
		rotation.value,
		context.mainCamera,
		context.config.settings.controlSensitivity
	);

	if (std::abs(mouseDir.x) >= 0.01f) {
		newRotation = utils::math::rotateAroundAxis(newRotation, upVector, -mouseDir.x * turnSpeedDt);
	}
	if (std::abs(mouseDir.y) >= 0.01f) {
		newRotation = utils::math::rotateAroundAxis(newRotation, rightVector, mouseDir.y * turnSpeedDt);
	}

	if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
		newRotation = utils::math::rotateAroundAxis(newRotation, upVector, -turnSpeedDt);
	if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
		newRotation = utils::math::rotateAroundAxis(newRotation, upVector, turnSpeedDt);
	if (IsKeyDown(KEY_UP))
		newRotation = utils::math::rotateAroundAxis(newRotation, rightVector, -turnSpeedDt);
	if (IsKeyDown(KEY_DOWN))
		newRotation = utils::math::rotateAroundAxis(newRotation, rightVector, turnSpeedDt);

	rotation.value = newRotation;
	const Vector3 forwardVector = utils::math::getForwardVector(rotation);

	const float accel = 50.0f;
	if (IsKeyDown(KEY_W)) {
		velocity.value += forwardVector * (accel * dt);
	} else if (IsKeyDown(KEY_S)) {
		velocity.value -= forwardVector * (accel * dt);
	}

	// Apply soft drag
	velocity.value = velocity.value * (1.0f - 0.5f * dt);

	// Arena clamp
	const float bound = context.config.ARENA_SIZE * 0.95f;
	position.value.x = Clamp(position.value.x, -bound, bound);
	position.value.y = Clamp(position.value.y, -bound, bound);
	position.value.z = Clamp(position.value.z, -bound, bound);
}
