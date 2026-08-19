#pragma once

#include "includes.hpp"

namespace utils::algorithm {
	Vector3 calculateVortexAttractionVelocity(
		const Vector3 &particlePos,
		const Vector3 &gravityPos,
		const Vector3 &gravityAxis,
		float strength,
		float maxRadius = 10.0f,
		float swirlRatio = 0.01f
	);
} // namespace utils::algorithm
