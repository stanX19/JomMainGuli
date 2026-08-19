#include "constants.hpp"
#include "utils.hpp"
#include "includes.hpp"
#include <algorithm>
#include <limits>
#include <cmath>

std::optional<utils::collision::CollisionInterval> utils::collision::calculateCollisionInterval(const Vector3 &posA, const Vector3 &velA, const Vector3 &posB, const Vector3 &velB, float collisionDistance)
{
	const Vector3 relPos = posA - posB;
	const Vector3 relVel = velA - velB;

	const float a = Vector3DotProduct(relVel, relVel);
	const float b = 2.0f * Vector3DotProduct(relPos, relVel);
	const float c = Vector3DotProduct(relPos, relPos) - collisionDistance * collisionDistance;

	const float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0.0f || a == 0.0f) {
		// overlapping or no collision
		if (c > constants::epsilon)
			return std::nullopt;
		return utils::collision::CollisionInterval{0.0f, std::numeric_limits<float>::infinity()};
	}

	const float sqrtD = sqrtf(discriminant);
	const float denominator = 2.0f * a;
	const float collisionStartDt = (-b - sqrtD) / denominator;
	const float collisionEndDt = (-b + sqrtD) / denominator;

	return utils::collision::CollisionInterval{collisionStartDt, collisionEndDt};
}

float utils::collision::calculateCollisionTime(const Vector3 &posA, const Vector3 &velA, const Vector3 &posB, const Vector3 &velB, float collisionDistance)
{
	const std::optional<utils::collision::CollisionInterval> interval = calculateCollisionInterval(
		posA,
		velA,
		posB,
		velB,
		collisionDistance
	);

	if (!interval)
		return -1.0f;

	if (interval->collisionEndDt < 0.0f)
		return interval->collisionEndDt; // collision was in the past
	if (interval->collisionStartDt < 0.0f)  // and t2 >= 0
		return 0.0f; // already overlapping
	return interval->collisionStartDt; // collision in future
}

bool utils::collision::willCollide(const std::optional<utils::collision::CollisionInterval> &interval, float maxDt)
{
	if (!interval || interval->collisionEndDt < 0.0f)
		return false;

	const float collisionDt = std::max(interval->collisionStartDt, 0.0f);
	return willCollide(collisionDt, maxDt);
}

bool utils::collision::willCollide(const Vector3 &posA, const Vector3 &velA,
                                   const Vector3 &posB, const Vector3 &velB,
                                   float collisionDistance, float maxDt)
{
	const std::optional<utils::collision::CollisionInterval> interval = calculateCollisionInterval(
		posA,
		velA,
		posB,
		velB,
		collisionDistance
	);
	return willCollide(interval, maxDt);
}

bool utils::collision::willCollide(float collisionDt, float maxDt)
{
	return (collisionDt >= 0.0f && collisionDt <= maxDt);
}

Vector3 utils::collision::calculateLeadDirection(
	const Vector3 &shooterPos,
	const Vector3 &targetPos,
	const Vector3 &targetVel,
	float projectileSpeed)
{
	const Vector3 toTarget = targetPos - shooterPos;
	const Vector3 relVel = targetVel;

	const float a = Vector3LengthSqr(relVel) - projectileSpeed * projectileSpeed;
	const float b = 2.0f * Vector3DotProduct(toTarget, relVel);
	const float c = Vector3LengthSqr(toTarget);

	const float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0 || fabsf(a) < 1e-5f)
	{
		return Vector3Normalize(toTarget);
	}

	const float sqrtDisc = sqrtf(discriminant);
	const float t1 = (-b + sqrtDisc) / (2.0f * a);
	const float t2 = (-b - sqrtDisc) / (2.0f * a);
	float interceptTime = fminf(t1, t2);

	if (interceptTime < 0.0f)
		interceptTime = fmaxf(t1, t2);

	if (interceptTime < 0.0f)
		return Vector3Normalize(toTarget);

	const Vector3 aimPos = targetPos + targetVel * interceptTime;
	return Vector3Normalize(aimPos - shooterPos);
}

Vector3 utils::collision::calculateVelocityBiasedDirection(
    const Vector3& chaserPos,
    const Vector3& targetPos,
    const Vector3& targetVel,
    float chaserSpeed
) {
	Vector3 finalDir = targetPos - chaserPos;

	for (int i = 0; i < 5; i++) {
		const float dist = Vector3Length(finalDir);
		const float chaseTime = dist / chaserSpeed;
		const Vector3 futureTargetPos = targetPos + targetVel * chaseTime;
		finalDir = futureTargetPos - chaserPos;
	}
	return Vector3Normalize(finalDir);
}

Vector3 utils::algorithm::calculateVortexAttractionVelocity(
	const Vector3 &particlePos,
	const Vector3 &gravityPos,
	const Vector3 &gravityAxis,
	float strength,
	float maxRadius,
	float swirlRatio)
{
	const Vector3 toTarget = gravityPos - particlePos;
	const float distSq = Vector3LengthSqr(toTarget);

	if (distSq > maxRadius * maxRadius || distSq < constants::epsilon)
		return Vector3{0.0f, 0.0f, 0.0f};

	const Vector3 toTargetNorm = Vector3Normalize(toTarget);
	const Vector3 gravitonVelNorm = Vector3Normalize(gravityAxis);
	const Vector3 displacedDir = Vector3CrossProduct(gravitonVelNorm, toTargetNorm);
	const float calculatedStrength = strength * Clamp(1.0f - (distSq / (maxRadius * maxRadius)), 0.1f, 1.0f);
	const Vector3 velChange = (toTargetNorm + displacedDir * swirlRatio) * calculatedStrength;

	return velChange;
}
