#include "mathUtils.hpp"
#include <cmath>
#include <cstdlib>

float utils::math::wrapAngle(float angle)
{
	while (angle < -PI)
		angle += 2.0f * PI;
	while (angle > PI)
		angle -= 2.0f * PI;
	return angle;
}

float utils::math::wrapAngleDegree(float angle)
{
	while (angle < -180.0f)
		angle += 2.0f * 180.0f;
	while (angle > 180.0f)
		angle -= 2.0f * 180.0f;
	return angle;
}

float utils::math::randomFloat(float min, float max)
{
	return min + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX) / (max - min));
}
