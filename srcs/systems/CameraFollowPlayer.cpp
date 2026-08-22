#include "systems/CameraFollowPlayer.hpp"
#include "utils.hpp"
#include <cmath>

using namespace component;

void systems::CameraFollowPlayer::update(GameContext &context, float dt) {
	if (!context.registry.valid(context.currentPlayer))
		return;

	auto [posPtr, rotPtr, colBodyPtr] = context.registry.try_get<Position, Rotation, CollisionBody>(context.currentPlayer);
	if (!posPtr || !rotPtr)
		return;

	UnitCamera *unitCamera = &m_defaultCamera;
	if (auto cameraComp = context.registry.try_get<UnitCamera>(context.currentPlayer))
		unitCamera = cameraComp;

	Position &pos = *posPtr;
	Rotation &rot = *rotPtr;
	Camera3D &camera = context.mainCamera;

	bool lookback = IsKeyDown(KEY_F);
	UnitCamera::POV pov = lookback ? unitCamera->lookBackPOV : unitCamera->mainPOV;

	float scale = colBodyPtr ? colBodyPtr->radius : 1.0f;

	Vector3 desiredPosition = Vector3RotateByQuaternion(pov.positionOffset, rot.value) * scale + pos.value;
	Vector3 desiredTarget = Vector3RotateByQuaternion(pov.targetOffset, rot.value) * scale + pos.value;

	float smoothing = unitCamera->lerpExp;
	float lerp = 1.0f - std::exp(-smoothing * dt);
	Vector3 up = utils::math::getUpVector(rot);

	camera.position = Vector3Lerp(camera.position, desiredPosition, lerp);
	camera.target = Vector3Lerp(camera.target, desiredTarget, lerp);
	camera.up = Vector3Lerp(camera.up, up, lerp * unitCamera->upLerpFactor);

	camera.fovy = pov.fovy;
	camera.projection = CAMERA_PERSPECTIVE;
}
