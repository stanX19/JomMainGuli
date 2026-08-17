#pragma once
#include "includes.hpp"
#include "components/physics.hpp"

namespace utils::math {
	Quaternion rotateAroundAxis(const Quaternion &current, const Vector3 &axis, float angle);
	Vector3 vector3Abs(const Vector3 &vec);
	Vector3 getForwardVector();
	Vector3 getForwardVector(const component::Rotation &rotation);
	Vector3 getForwardVector(const Quaternion &rotation);
	Vector3 getRightVector();
	Vector3 getRightVector(const component::Rotation &rotation);
	Vector3 getRightVector(const Quaternion &rotation);
	Vector3 getUpVector();
	Vector3 getUpVector(const component::Rotation &rotation);
	Vector3 getUpVector(const Quaternion &rotation);
	Quaternion vector3ToRotation(const Vector3 &forward);
	Quaternion vector3ToRotation(const Vector3 &forward, const Vector3 &up);
	Quaternion vector3ToRotation(const Vector3 &newForward, const Quaternion &baseRotation);
	Vector3 randomUnitVector3();
	Quaternion randomRotation();
	Matrix getTransformMatrix(const Vector3 &scale, const Vector3 &rotation, const Vector3 &displacement);

	float angleDifference(const Vector3 &a, const Vector3 &b);
	float angleDifference(const Quaternion &a, const Quaternion &b);
	float angleDifference(const component::Rotation &a, const component::Rotation &b);
	float angleDifference(const Quaternion &a, const component::Rotation &b);
	float angleDifference(const component::Rotation &a, const Quaternion &b);

	float wrapAngle(float angle);
	float wrapAngleDegree(float angle);
	float randomFloat(float min = -1.0f, float max = 1.0f);
} // namespace utils::math