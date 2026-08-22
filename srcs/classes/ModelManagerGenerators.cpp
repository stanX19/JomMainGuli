#include "ModelManager.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
	using Vertex = ModelManager::Vertex;
	using Triangle = ModelManager::Triangle;

	void addTriangle(std::vector<Triangle> &triangles, Vertex v0, Vertex v1, Vertex v2) {
		triangles.push_back({v0, v1, v2});
	}

	// Single-sided rect (2 front triangles with colors)
	void addSingleSidedRectWithColor(
		std::vector<Triangle> &triangles,
		Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3,
		Vector3 normal,
		Color c0, Color c1, Color c2, Color c3,
		Vector2 uv0 = {0.0f, 0.0f}, Vector2 uv1 = {0.0f, 0.0f},
		Vector2 uv2 = {0.0f, 0.0f}, Vector2 uv3 = {0.0f, 0.0f}
	) {
		// 0 3
		// 1 2
		// Front face: 0-1-2, 0-2-3
		addTriangle(triangles, {v0, normal, c0, uv0}, {v1, normal, c1, uv1}, {v2, normal, c2, uv2});
		addTriangle(triangles, {v0, normal, c0, uv0}, {v2, normal, c2, uv2}, {v3, normal, c3, uv3});
	}

	// Double-sided rect (no culling: 2 front triangles + 2 back triangles)
	void addDoubleSidedRect(
		std::vector<Triangle> &triangles,
		Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3,
		Vector3 normal, Color color,
		Vector2 uv0 = {0.0f, 0.0f}, Vector2 uv1 = {0.0f, 0.0f},
		Vector2 uv2 = {0.0f, 0.0f}, Vector2 uv3 = {0.0f, 0.0f}
	) {
		// 0 3
		// 1 2
 		// Front face, 0-1-2, 0-2-3
		addTriangle(triangles, {v0, normal, color, uv0}, {v1, normal, color, uv1}, {v2, normal, color, uv2});
		addTriangle(triangles, {v0, normal, color, uv0}, {v2, normal, color, uv2}, {v3, normal, color, uv3});

		// Back face, 2-1-0, 3-2-0
		const Vector3 backNormal = Vector3Scale(normal, -1.0f);
		addTriangle(triangles, {v2, backNormal, color, uv2}, {v1, backNormal, color, uv1}, {v0, backNormal, color, uv0});
		addTriangle(triangles, {v3, backNormal, color, uv3}, {v2, backNormal, color, uv2}, {v0, backNormal, color, uv0});
	}
} // namespace

ModelId ModelManager::loadModelFromImage(const std::string &imagePath, float quadSize) {
	const std::string key = generateCacheKey("image", imagePath, quadSize);
	const auto it = m_proceduralCache.find(key);
	if (it != m_proceduralCache.end()) {
		return it->second;
	}

	Texture2D texture = LoadTexture(imagePath.c_str());
	if (texture.id == 0) {
		TraceLog(LOG_WARNING, "Failed to load image: %s", imagePath.c_str());
		return createSphere(16, 16, quadSize * 0.5f);
	}

	SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);

	const float s = quadSize * 0.5f;
	std::vector<Triangle> triangles;
	triangles.reserve(4);

	addDoubleSidedRect(
		triangles,
		{-s, -s, 0.0f}, { s, -s, 0.0f}, { s,  s, 0.0f}, {-s,  s, 0.0f},
		{0.0f, 0.0f, 1.0f}, WHITE,
		{0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}
	);

	Model model = LoadModelFromMesh(generateMeshFromTriangles(triangles, true));
	model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
	model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

	const ModelId id = m_models.size();
	m_models.push_back(model);
	m_modelRadii.push_back(computeModelBoundingRadius(model));
	m_modelPaths.emplace_back(imagePath);
	m_proceduralCache[key] = id;
	return id;
}

namespace {
	float calculateRungWidth(float baseRad, float t) {
		const float distFromMiddle = std::abs(t - 0.5f) / 0.5f;
		return baseRad * (1.0f - distFromMiddle);
	}
}

ModelId ModelManager::createRibbon(
	const std::vector<Color> &colors,
	float height,
	float width,
	float twistTurns
) {
	std::stringstream ss;
	ss << "ribbon";
	for (const auto &c : colors) {
		ss << "_" << static_cast<int>(c.r) << "-" << static_cast<int>(c.g) << "-" << static_cast<int>(c.b);
	}
	const std::string key = generateCacheKey(ss.str(), height, width, twistTurns);
	const auto it = m_proceduralCache.find(key);
	if (it != m_proceduralCache.end()) {
		return it->second;
	}
	const std::vector<Color> strandColors = colors.empty() ? std::vector<Color>{WHITE} : colors;
	const size_t strandCount = strandColors.size();

	constexpr int RUNGS = 16;
	const float rad = width * 0.5f;
	std::vector<Triangle> triangles;
	triangles.reserve(RUNGS * 4 * strandCount);

	for (size_t i = 0; i < strandCount; ++i) {
		const float baseAngleOffset = (2.0f * PI * static_cast<float>(i)) / static_cast<float>(strandCount);
		const Color strandColor = strandColors[i];

		for (int k = 0; k < RUNGS; ++k) {
			// t: Normalised for k and k+1 in %, [0.0, 1.0]
			const float t0 = static_cast<float>(k) / static_cast<float>(RUNGS);
			const float t1 = static_cast<float>(k + 1) / static_cast<float>(RUNGS);
			
			// thin-thick-thin radius
			const float currRungRad = calculateRungWidth(rad, t0);
			const float nextRungRad = calculateRungWidth(rad, t1);

			// y: height of slice
			const float y0 = -height * 0.5f + height * t0;
			const float y1 = -height * 0.5f + height * t1;

			// a: angle of slice
			const float a0 = baseAngleOffset + (t0 - 0.5f) * twistTurns * 2.0f * PI;
			const float a1 = baseAngleOffset + (t1 - 0.5f) * twistTurns * 2.0f * PI;

			// u: Center to slice's outer edge
			const Vector3 u0 = {std::cos(a0) * currRungRad, 0.0f, std::sin(a0) * currRungRad};
			const Vector3 u1 = {std::cos(a1) * nextRungRad, 0.0f, std::sin(a1) * nextRungRad};
			const Vector3 norm = {-std::sin(a0), 0.0f, std::cos(a0)}; // norm of slice

			// l, r: Left and right of current slice (0) and next slice (1)
			const Vector3 l0 = {-u0.x, y0, -u0.z};
			const Vector3 r0 = { u0.x, y0,  u0.z};

			const Vector3 l1 = {-u1.x, y1, -u1.z};
			const Vector3 r1 = { u1.x, y1,  u1.z};

			addDoubleSidedRect(triangles, l0, l1, r1, r0, norm, strandColor);
		}
	}
	Model model = LoadModelFromMesh(generateMeshFromTriangles(triangles, false));

	const ModelId id = m_models.size();
	m_models.push_back(model);
	m_modelRadii.push_back(computeModelBoundingRadius(model));
	m_modelPaths.emplace_back(std::nullopt);
	m_proceduralCache[key] = id;
	return id;
}

ModelId ModelManager::createHoleCylinder(
	float radius,
	float height,
	int steps,
	int facesPerCircle,
	float startOpacity
) {
	const std::string key = generateCacheKey(
		"hole_cylinder", radius, height, steps, facesPerCircle, startOpacity
	);
	const auto it = m_proceduralCache.find(key);
	if (it != m_proceduralCache.end()) {
		return it->second;
	}

	if (facesPerCircle < 3) {
		facesPerCircle = 3;
	}
	if (steps < 1) {
		steps = 1;
	}

	const float angleStep = (2.0f * PI) / facesPerCircle;
	std::vector<Triangle> triangles;
	triangles.reserve(steps * facesPerCircle * 2); // 1 face 2 triangle

	for (int s = 0; s < steps; ++s) {
		const float t0 = static_cast<float>(s) / static_cast<float>(steps);
		const float t1 = static_cast<float>(s + 1) / static_cast<float>(steps);

		const float y0 = height * t0;
		const float y1 = height * t1;

		const float alpha0 = std::clamp(startOpacity * (1.0f - t0), 0.0f, 1.0f);
		const float alpha1 = std::clamp(startOpacity * (1.0f - t1), 0.0f, 1.0f);

		const Color color0 = ColorAlpha(WHITE, alpha0);
		const Color color1 = ColorAlpha(WHITE, alpha1);

		for (int i = 0; i < facesPerCircle; ++i) {
			// 0 = left; 1 = right;
			const float angle0 = static_cast<float>(i) * angleStep;
			const float angle1 = static_cast<float>(i + 1) * angleStep;

			const float sin0 = std::sin(angle0) * radius;
			const float cos0 = std::cos(angle0) * radius;
			const float sin1 = std::sin(angle1) * radius;
			const float cos1 = std::cos(angle1) * radius;

			// from outside
			// 3----2
			// |    |
			// 0----1
			const Vector3 v0 = {sin0, y0, cos0};
			const Vector3 v1 = {sin1, y0, cos1};
			const Vector3 v2 = {sin1, y1, cos1};
			const Vector3 v3 = {sin0, y1, cos0};

			const float midAngle = (angle0 + angle1) * 0.5f;
			const Vector3 normal = {std::sin(midAngle), 0.0f, std::cos(midAngle)};

			addSingleSidedRectWithColor(
				triangles,
				v0, v1, v2, v3,
				normal,
				color0, color0, color1, color1
			);
		}
	}

	Model model = LoadModelFromMesh(generateMeshFromTriangles(triangles, false));
	const ModelId id = m_models.size();
	m_models.push_back(model);
	m_modelRadii.push_back(computeModelBoundingRadius(model));
	m_modelPaths.emplace_back(std::nullopt);
	m_proceduralCache[key] = id;
	return id;
}

