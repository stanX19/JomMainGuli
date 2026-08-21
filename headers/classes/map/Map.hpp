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

		static Color getRaylibColor(ColorType type);
		static ColorType getColorType(Color color);

		const TileData& getGridData(Vector3 worldCords) const;
		const TileData& getTile(int x, int y) const;
		const TileData& getTile(CellCord cord) const;
		bool isValidGrid(int x, int y) const;
		bool isValidGrid(CellCord cord) const;
		CellCord worldToGrid(Vector3 worldPos) const;

		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }
		float getTileSize() const { return m_tileSize; }
		float getTileUnitHeight() const { return m_tileUnitHeight; }
		int getSmoothingHeightDiff() const { return m_smoothingHeightDiff; }

		std::optional<ModelId> getModelId() const { return m_modelId; }
		const std::vector<CellCord>& getHoleCords() const { return m_holeCords; }
		float getHoleRadius() const { return m_tileSize * 0.5f * m_holeRadiusRatio; }
		float getHoleRadiusPercent() const { return m_holeRadiusRatio; }
		bool isWithinHole(Vector3 worldPos) const;
		std::optional<CellCord> getHoleCellIfInHole(Vector3 worldPos) const;
		Vector3 getHoleCenter(CellCord cell) const;
		const std::vector<InitialEntity>& getInitialEntities() const { return m_initialEntities; }
		const std::vector<std::vector<TileData>>& getGrid() const { return m_grid; }

		void spawnAll(GameContext &context) const;
		Vector3 gridToWorld(int x, int y) const;
		Vector3 gridToWorld(CellCord cellCords) const;
		float getGroundY(Vector3 worldPos, Vector3 *outNormal = nullptr) const;

	private:
		friend class MapLoader;
		friend class MapMeshGenerator;

		void init(int width, int height, float tileSize, float tileUnitHeight, float holeRadiusRatio = 0.6f, int smoothingHeightDiff = 2);
		void setTile(int x, int y, const TileData &tile);
		void addInitialEntity(const InitialEntity &entity);
		void setHoleCords(const std::vector<CellCord> &cords) { m_holeCords = cords; }
		void addHoleCord(CellCord cord) { m_holeCords.push_back(cord); }
		void setModelId(ModelId id) { m_modelId = id; }
		void populateTileBounds();
		float computeCornerForTile(int tx, int ty, int cx, int cz) const;
		void clear();

		int m_width = 0;
		int m_height = 0;
		float m_tileSize = 2.0f;
		float m_tileUnitHeight = 1.0f;
		float m_holeRadiusRatio = 0.6f;
		int m_smoothingHeightDiff = 2;
		std::vector<std::vector<TileData>> m_grid;
		std::vector<InitialEntity> m_initialEntities;
		std::vector<CellCord> m_holeCords;
		std::optional<ModelId> m_modelId = std::nullopt;
	};
} // namespace map
