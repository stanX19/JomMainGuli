#include "map/MapMeshGenerator.hpp"
#include <vector>

namespace {
	Color shadeColor(Color c, float factor) {
		return Color{
			static_cast<unsigned char>(c.r * factor),
			static_cast<unsigned char>(c.g * factor),
			static_cast<unsigned char>(c.b * factor),
			c.a
		};
	}
}

map::MapMeshGenerator::MapMeshGenerator(Map &map)
	: m_map(map),
	  m_width(map.getWidth()),
	  m_height(map.getHeight()),
	  m_tileSize(map.getTileSize()),
	  m_tileUnitHeight(map.getTileUnitHeight()),
	  m_halfWidth((m_width * m_tileSize) * 0.5f),
	  m_halfHeight((m_height * m_tileSize) * 0.5f),
	  m_baseBottom(-2.0f * m_tileUnitHeight)
{}

void map::MapMeshGenerator::addTriangle(
	Vector3 v0, Vector3 v1, Vector3 v2,
	Vector3 normal,
	Color color
) {
	m_triangles.push_back({
		Vertex{v0, normal, color, {0.0f, 0.0f}},
		Vertex{v1, normal, color, {0.0f, 0.0f}},
		Vertex{v2, normal, color, {0.0f, 0.0f}}
	});
}

// Rect = 2 triangle
// 0---3
// | \ |
// 1---2
// Vertices must be provided in counter-clockwise order cuz of culling
void map::MapMeshGenerator::addRect(
	Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3,
	Vector3 normal,
	Color color
) {
    // [0, 1, 2], [0, 2, 3]
	addTriangle(v0, v1, v2, normal, color);
	addTriangle(v0, v2, v3, normal, color);
}

void map::MapMeshGenerator::addTileTop(
	float x0, float x1, float z0, float z1,
	float yTop,
	Color color
) {
	const Vector3 top0 = {x0, yTop, z0};
	const Vector3 top1 = {x0, yTop, z1};
	const Vector3 top2 = {x1, yTop, z1};
	const Vector3 top3 = {x1, yTop, z0};
	addRect(top0, top1, top2, top3, Vector3{0.0f, 1.0f, 0.0f}, color);
}

/* 
               z0 (North)
            top0 ------ top3
              |          |
 x0 (West)    |   Tile   |   x1 (East)
              |  (x, y)  |
              |          |
            top1 ------ top2
               z1 (South)
*/
void map::MapMeshGenerator::addTileSides(
	int x, int y,
	float x0, float x1, float z0, float z1,
	float yTop,
	Color color
) {
	// North (y - 1): z0 face
	const float northY = (y > 0) ? (m_map.getTile(x, y - 1).height * m_tileUnitHeight) : m_baseBottom;
	if (yTop > northY) {
		const Vector3 n0 = {x1, yTop, z0};
		const Vector3 n1 = {x1, northY, z0};
		const Vector3 n2 = {x0, northY, z0};
		const Vector3 n3 = {x0, yTop, z0};
		addRect(n0, n1, n2, n3, Vector3{0.0f, 0.0f, -1.0f}, shadeColor(color, 0.85f));
	}

	// South (y + 1): z1 face
	const float southY = (y + 1 < m_height) ? (m_map.getTile(x, y + 1).height * m_tileUnitHeight) : m_baseBottom;
	if (yTop > southY) {
		const Vector3 s0 = {x0, yTop, z1};
		const Vector3 s1 = {x0, southY, z1};
		const Vector3 s2 = {x1, southY, z1};
		const Vector3 s3 = {x1, yTop, z1};
		addRect(s0, s1, s2, s3, Vector3{0.0f, 0.0f, 1.0f}, shadeColor(color, 0.85f));
	}

	// West (x - 1): x0 face
	const float westY = (x > 0) ? (m_map.getTile(x - 1, y).height * m_tileUnitHeight) : m_baseBottom;
	if (yTop > westY) {
		const Vector3 w0 = {x0, yTop, z0};
		const Vector3 w1 = {x0, westY, z0};
		const Vector3 w2 = {x0, westY, z1};
		const Vector3 w3 = {x0, yTop, z1};
		addRect(w0, w1, w2, w3, Vector3{-1.0f, 0.0f, 0.0f}, shadeColor(color, 0.75f));
	}

	// East (x + 1): x1 face
	const float eastY = (x + 1 < m_width) ? (m_map.getTile(x + 1, y).height * m_tileUnitHeight) : m_baseBottom;
	if (yTop > eastY) {
		const Vector3 e0 = {x1, yTop, z1};
		const Vector3 e1 = {x1, eastY, z1};
		const Vector3 e2 = {x1, eastY, z0};
		const Vector3 e3 = {x1, yTop, z0};
		addRect(e0, e1, e2, e3, Vector3{1.0f, 0.0f, 0.0f}, shadeColor(color, 0.75f));
	}
}

Mesh map::MapMeshGenerator::generateMesh() {
	if (m_width <= 0 || m_height <= 0) {
		return Mesh{};
	}

	m_triangles.clear();
	m_triangles.reserve(m_width * m_height * 12);

	for (int y = 0; y < m_height; ++y) {
		for (int x = 0; x < m_width; ++x) {
			const TileData &tile = m_map.getTile(x, y);
			const Color tileColor = ColorLerp(GRAY, Map::getRaylibColor(tile.color), 0.25);

			const float x0 = x * m_tileSize - m_halfWidth;
			const float x1 = (x + 1) * m_tileSize - m_halfWidth;
			const float z0 = y * m_tileSize - m_halfHeight;
			const float z1 = (y + 1) * m_tileSize - m_halfHeight;
			const float yTop = tile.height * m_tileUnitHeight;

			addTileTop(x0, x1, z0, z1, yTop, tileColor);
			addTileSides(x, y, x0, x1, z0, z1, yTop, tileColor);
		}
	}

	return ModelManager::generateMeshFromTriangles(m_triangles, false);
}

void map::MapMeshGenerator::generateAndAssignModel(ModelManager &modelManager) {
	const Mesh mesh = generateMesh();
	const ModelId id = modelManager.loadModel(mesh);
	m_map.setModelId(id);
}
