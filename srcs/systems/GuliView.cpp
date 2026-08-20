#include "systems/GuliView.hpp"
#include "components.hpp"
#include "ui/GameHUD.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cmath>

using namespace component;

namespace {
	void handleInput(GameContext &context, GuliInventory &inventory) {
		const entt::entity player = context.currentPlayer;
		auto *viewState = context.registry.try_get<GuliViewState>(player);

		if (viewState && (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))) {
			context.registry.remove<GuliViewState>(player);
			return;
		}

		for (int key = KEY_ONE; key <= KEY_NINE; ++key) {
			if (!IsKeyPressed(key))
				continue;

			const size_t slot = static_cast<size_t>(key - KEY_ONE);
			if (slot >= inventory.guliCollection.size())
				continue;

			if (viewState && viewState->selectedIndex == slot) {
				context.registry.remove<GuliViewState>(player);
				return;
			}

			const entt::entity target = inventory.guliCollection[slot];
			context.registry.emplace_or_replace<GuliViewState>(player, GuliViewState{slot, target});
			return;
		}
	}

	void positionInspectedGuli(entt::registry &registry, entt::entity target, const Camera3D &camera, const Vector3 &up, const Vector3 &right, float dt) {
		if (!registry.valid(target))
			return;

		auto [pos, rb, rot] = registry.try_get<Position, RenderBody, Rotation>(target);
		if (!pos || !rb || !rot)
			return;

		const int screenWidth = GetScreenWidth();
		const int screenHeight = GetScreenHeight();
		const Vector2 centerPos = {static_cast<float>(screenWidth) * 0.5f, static_cast<float>(screenHeight) * 0.42f};


		constexpr float DIST = 2.5f;
		const Ray ray = GetScreenToWorldRay(centerPos, camera);
		pos->value = ray.position + ray.direction * DIST;

		const float fovy = (camera.fovy > 0.0f) ? camera.fovy : 45.0f;
		const float halfHeight = std::max(static_cast<float>(screenHeight) * 0.5f, 1.0f);
		const float targetPixelRadius = 120.0f;
		const float sphereScale = (targetPixelRadius * DIST * std::tan(fovy * 0.5f * DEG2RAD)) / halfHeight;
		rb->scale = Vector3{sphereScale, sphereScale, sphereScale};


		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			const Vector2 mouseDelta = GetMouseDelta();
			if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
				const Quaternion yaw = QuaternionFromAxisAngle(up, -mouseDelta.x * 0.008f);
				const Quaternion pitch = QuaternionFromAxisAngle(right, -mouseDelta.y * 0.008f);
				rot->value = QuaternionNormalize(QuaternionMultiply(QuaternionMultiply(yaw, pitch), rot->value));
				return;
			}
		}

		const Quaternion idleSpin = QuaternionFromAxisAngle(up, 0.6f * dt);
		rot->value = QuaternionNormalize(QuaternionMultiply(idleSpin, rot->value));
	}

	void positionInventoryGuli(entt::registry &registry, entt::entity guli, size_t index, const Camera3D &camera, float dt) {
		if (!registry.valid(guli))
			return;

		auto [pos, rb, rot] = registry.try_get<Position, RenderBody, Rotation>(guli);
		if (!pos || !rb || !rot)
			return;

		const int screenWidth = GetScreenWidth();
		const int screenHeight = GetScreenHeight();
		const Vector2 previewCenter = ui::GameHUD::getSlotPreviewCenter(index, screenWidth);

		constexpr float DIST = 2.0f;
		const Ray ray = GetScreenToWorldRay(previewCenter, camera);
		pos->value = ray.position + ray.direction * DIST;

		const float fovy = (camera.fovy > 0.0f) ? camera.fovy : 45.0f;
		const float halfHeight = std::max(static_cast<float>(screenHeight) * 0.5f, 1.0f);
		const float targetPixelRadius = 16.0f;
		const float sphereScale = (targetPixelRadius * DIST * std::tan(fovy * 0.5f * DEG2RAD)) / halfHeight;
		rb->scale = Vector3{sphereScale, sphereScale, sphereScale};


		const Quaternion idleSpin = QuaternionFromAxisAngle(camera.up, 1.0f * dt);
		rot->value = QuaternionNormalize(QuaternionMultiply(idleSpin, rot->value));
	}
}

void systems::GuliView::update(GameContext &context, float dt) {
	if (!context.registry.valid(context.currentPlayer))
		return;

	auto *inventoryPtr = context.registry.try_get<GuliInventory>(context.currentPlayer);
	if (!inventoryPtr)
		return;

	handleInput(context, *inventoryPtr);

	const Camera3D &camera = context.mainCamera;
	const Vector3 forward = Vector3Normalize(camera.target - camera.position);
	const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
	const Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

	const auto *viewStatePtr = context.registry.try_get<GuliViewState>(context.currentPlayer);

	for (size_t i = 0; i < inventoryPtr->guliCollection.size(); ++i) {
		const entt::entity guli = inventoryPtr->guliCollection[i];

		if (viewStatePtr && viewStatePtr->selectedIndex == i) {
			positionInspectedGuli(context.registry, guli, camera, up, right, dt);
			continue;
		}
		positionInventoryGuli(context.registry, guli, i, camera, dt);
	}
}
