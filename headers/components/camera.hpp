#pragma once

#include "includes.hpp"

namespace component {
	struct UnitCamera {
		struct POV {
			Vector3 positionOffset = {0.0f, 4.0f, -10.0f};
			Vector3 targetOffset = {0.0f, 2.0f, 10.0f};
			float fovy = 45.0f;
		};

		POV mainPOV = POV{
			.positionOffset = {0.0f, 4.0f, -10.0f},
			.targetOffset = {0.0f, 2.0f, 10.0f},
			.fovy = 45.0f
		};
		POV aimPOV = POV{
			.positionOffset = {0.0f, 0.0f, 50.0f},
			.targetOffset = {0.0f, 0.0f, 1000.0f},
			.fovy = 30.0f
		};
		POV lookBackPOV = POV{
			.positionOffset = {0.0f, 4.0f, 10.0f},
			.targetOffset = {0.0f, 2.0f, -10.0f},
			.fovy = 90.0f
		};

		float lerpExp = 12.0f;
		float upLerpFactor = 0.2f;
		bool isAiming = false;
	};
} // namespace component
