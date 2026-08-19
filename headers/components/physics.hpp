#pragma once

#include "includes.hpp"

namespace component {
	struct Position {
		Vector3 value = {0.0f, 0.0f, 0.0f};
	};

	struct PrevPosition {
		Vector3 value = {0.0f, 0.0f, 0.0f};
	};

	struct Velocity {
		Vector3 value = {0.0f, 0.0f, 0.0f};
	};

	struct ScalarAcceleration {
		float value = 0.0f;
	};

	struct Rotation {
		Quaternion value = QuaternionIdentity();
	};

	struct PrevRotation {
		Quaternion value = QuaternionIdentity();
	};

	struct RotationVelocity {
		Quaternion value = QuaternionIdentity();
	};

	struct Mass {
		float value = 1.0f;
	};

	// -1.0f -100% gravity
	//  0.0f no gravity
	//  1.0f 100% gravity
	struct VerticalGravity {
		float value = 1.0f;
	};

	namespace tags {
		struct zeroGravity {};
	}
} // namespace component
