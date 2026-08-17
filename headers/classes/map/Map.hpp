#pragma once

#include "includes.hpp"
#include "ModelManager.hpp"
#include "map/MapTypes.hpp"
#include <vector>
#include <optional>
#include <cstdint>

struct GameContext;

namespace map {
	class MapLoader;
	class MapMeshGenerator;

	class Map {
	public:
		Map();
		~Map() = default;

		static Color toRaylibColor(ColorType type);

		const TileData& getGridData(Vector3 worldCords) const;
		const TileData& getTile(int x, int y) const;
		const TileData& getTile(CellCord cord) const;
		bool isValidGrid(int x, int y) const;
		bool isValidGrid(CellCord cord) const;
		CellCord worldToGrid(Vector3 worldPos) const;

		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }
		float getTileSize() const { return m_tileSize; }
		float getHeightScale() const { return m_heightScale; }

		std::optional<ModelId> getModelId() const { return m_modelId; }
		CellCord getFlagCords() const { return m_flagCords; }
		const std::vector<InitialEntity>& getInitialEntities() const { return m_initialEntities; }
		const std::vector<std::vector<TileData>>& getGrid() const { return m_grid; }

		void spawnAll(GameContext &context) const;
		Vector3 gridToWorld(int x, int y) const;
		Vector3 gridToWorld(CellCord cellCords) const;

	private:
		friend class MapLoader;
		friend class MapMeshGenerator;

		void init(int width, int height, float tileSize, float heightScale);
		void setTile(int x, int y, const TileData &tile);
		void addInitialEntity(const InitialEntity &entity);
		void setFlagCords(CellCord cords) { m_flagCords = cords; }
		void setModelId(ModelId id) { m_modelId = id; }
		void populateTileBounds();
		void clear();

		int m_width = 0;
		int m_height = 0;
		float m_tileSize = 2.0f;
		float m_heightScale = 1.0f;
		std::vector<std::vector<TileData>> m_grid;
		std::vector<InitialEntity> m_initialEntities;
		CellCord m_flagCords = {0, 0};
		std::optional<ModelId> m_modelId = std::nullopt;
	};
} // namespace map
