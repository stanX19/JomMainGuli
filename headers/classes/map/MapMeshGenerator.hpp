#pragma once

#include "includes.hpp"
#include "map/Map.hpp"
#include "ModelManager.hpp"
#include <vector>

namespace map {
	class MapMeshGenerator {
	public:
		explicit MapMeshGenerator(Map &map);
		~MapMeshGenerator() = default;

		Mesh generateMesh();
		void generateAndAssignModel(ModelManager &modelManager);

		using Vertex = ModelManager::Vertex;
		using Triangle = ModelManager::Triangle;

		Map &m_map;
		int m_width = 0;
		int m_height = 0;
		float m_tileSize = 10.0f;
		float m_tileUnitHeight = 1.0f;
		float m_halfWidth = 0.0f;
		float m_halfHeight = 0.0f;
		float m_baseBottom = -2.0f;

		std::vector<Triangle> m_triangles;

		void addTriangle(
			Vector3 v0, Vector3 v1, Vector3 v2,
			Vector3 normal,
			Color color
		);

		void addRect(
			Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3,
			Vector3 normal,
			Color color
		);

		void addTileTop(
			const TileData &tile,
			Color color
		);

		void addTileSides(
			int x, int y,
			const TileData &tile,
			Color color
		);
	};
} // namespace map
