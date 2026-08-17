#pragma once

#include "includes.hpp"

class Frustum {
public:
	struct Plane {
		float a = 0.0f;
		float b = 0.0f;
		float c = 0.0f;
		float d = 0.0f;
	};

	Frustum() = default;

	static Frustum fromViewProjection(const Matrix &viewProj);

	bool isSphereInside(const Vector3 &center, float radius) const;
	bool isPointInside(const Vector3 &point) const;

private:
	Plane m_planes[6]{}; // Left, Right, Bottom, Top, Near, Far
};
