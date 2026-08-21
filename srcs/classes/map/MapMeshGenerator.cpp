#include "map/MapMeshGenerator.hpp"
#include <vector>
#include <array>

namespace {
	Color shadeColor(Color c, float factor) {
		return Color{
			static_cast<unsigned char>(c.r * factor),
			static_cast<unsigned char>(c.g * factor),
			static_cast<unsigned char>(c.b * factor),
			c.a
		};
	}

	struct SideConfig {
		int dx;
		int dy;
		Vector3 normal;
		float shade;
		Vector3 topA;
		Vector3 topB;
		Vector3 bottomA;
		Vector3 bottomB;
	};

	std::array<SideConfig, 4> getTileSideConfigs(const map::TileData &tile, float baseBottom) {
		return {{
			// North (z0 edge)
			{
				0, -1,
				{0.0f, 0.0f, -1.0f},
				0.85f,
			    {tile.x2, tile.cornerY.x1z0, tile.z1},
				{tile.x1, tile.cornerY.x0z0, tile.z1},
				{tile.x2, baseBottom, tile.z1},
				{tile.x1, baseBottom, tile.z1}
			},
			// South (z1 edge)
			{
				0, 1,
				{0.0f, 0.0f, 1.0f},
				0.85f,
				{tile.x1, tile.cornerY.x0z1, tile.z2},
				{tile.x2, tile.cornerY.x1z1, tile.z2},
				{tile.x1, baseBottom, tile.z2},
				{tile.x2, baseBottom, tile.z2}
			},
			// West (x0 edge)
			{
				-1, 0,
				{-1.0f, 0.0f, 0.0f},
				0.85f,
				{tile.x1, tile.cornerY.x0z0, tile.z1},
				{tile.x1, tile.cornerY.x0z1, tile.z2},
				{tile.x1, baseBottom, tile.z1},
				{tile.x1, baseBottom, tile.z2}
			},
			// East (x1 edge)
			{
				1, 0,
				{1.0f, 0.0f, 0.0f},
				0.85f,
				{tile.x2, tile.cornerY.x1z1, tile.z2},
				{tile.x2, tile.cornerY.x1z0, tile.z1},
				{tile.x2, baseBottom, tile.z2},
				{tile.x2, baseBottom, tile.z1}
			}
		}};
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

/*
	v00---v10  
	|  \ /  |    
	|   c   |   
	|  / \  | 
	v01---v11

	CCW rotation: v00 c v10; v01 c v00; v11 c v01; v10 c v11
*/
void map::MapMeshGenerator::addTileTop(
	const TileData &tile,
	Color color
) {
	const Vector3 v00 = {tile.x1, tile.cornerY.x0z0, tile.z1};
	const Vector3 v10 = {tile.x2, tile.cornerY.x1z0, tile.z1};
	const Vector3 v01 = {tile.x1, tile.cornerY.x0z1, tile.z2};
	const Vector3 v11 = {tile.x2, tile.cornerY.x1z1, tile.z2};
	const Vector3 c = {tile.xMid, tile.yMid, tile.zMid};

	addTriangle(v00, c, v10, tile.normals.z0, color);
	addTriangle(v01, c, v00, tile.normals.x0, color);
	addTriangle(v11, c, v01, tile.normals.z1, color);
	addTriangle(v10, c, v11, tile.normals.x1, color);
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
	const TileData &tile,
	Color color
) {
	const auto sideConfigs = getTileSideConfigs(tile, m_baseBottom);

	for (std::size_t i = 0; i < sideConfigs.size(); ++i) {
		const auto &side = sideConfigs[i];
		const int nx = x + side.dx;
		const int ny = y + side.dy;

		if (!m_map.isValidGrid(nx, ny)) {
			addRect(side.topA, side.bottomA, side.bottomB, side.topB, side.normal, shadeColor(color, side.shade));
			continue;
		}

		const TileData &neighbor = m_map.getTile(nx, ny);
		if (tile.height - neighbor.height < m_map.getSmoothingHeightDiff())
			continue;

		Vector3 bottomA = side.bottomA;
		Vector3 bottomB = side.bottomB;
		if (i == 0) {
			bottomA.y = neighbor.cornerY.x1z1;
			bottomB.y = neighbor.cornerY.x0z1;
		} else if (i == 1) {
			bottomA.y = neighbor.cornerY.x0z0;
			bottomB.y = neighbor.cornerY.x1z0;
		} else if (i == 2) {
			bottomA.y = neighbor.cornerY.x1z0;
			bottomB.y = neighbor.cornerY.x1z1;
		} else {
			bottomA.y = neighbor.cornerY.x0z1;
			bottomB.y = neighbor.cornerY.x0z0;
		}

		addRect(side.topA, bottomA, bottomB, side.topB, side.normal, shadeColor(color, side.shade));
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

			addTileTop(tile, tileColor);
			addTileSides(x, y, tile, tileColor);
		}
	}

	return ModelManager::generateMeshFromTriangles(m_triangles, false);
}

void map::MapMeshGenerator::generateAndAssignModel(ModelManager &modelManager) {
	const Mesh mesh = generateMesh();
	const ModelId id = modelManager.loadModel(mesh);
	m_map.setModelId(id);
}
