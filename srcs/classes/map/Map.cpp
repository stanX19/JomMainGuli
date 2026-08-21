#include "constants.hpp"
#include "utils.hpp"
#include "map/Map.hpp"
#include "GameContext.hpp"
#include "entities.hpp"
#include <cmath>

Color map::Map::getRaylibColor(ColorType type) {
	switch (type) {
		case ColorType::Red:
			return RED;
		case ColorType::Green:
			return GREEN;
		case ColorType::Blue:
			return BLUE;
		case ColorType::Yellow:
			return YELLOW;
		case ColorType::Purple:
			return PURPLE;
		case ColorType::Orange:
			return ORANGE;
		case ColorType::White:
			return WHITE;
		case ColorType::None:
		default:
			return GRAY;
	}
}

map::ColorType map::Map::getColorType(Color type) {
	if (ColorIsEqual(type, RED)) return ColorType::Red;
	if (ColorIsEqual(type, GREEN)) return ColorType::Green;
	if (ColorIsEqual(type, BLUE)) return ColorType::Blue;
	if (ColorIsEqual(type, YELLOW)) return ColorType::Yellow;
	if (ColorIsEqual(type, PURPLE)) return ColorType::Purple;
	if (ColorIsEqual(type, ORANGE)) return ColorType::Orange;
	if (ColorIsEqual(type, WHITE)) return ColorType::White;
	return ColorType::None;
}

map::Map::Map() = default;

void map::Map::init(int width, int height, float tileSize, float tileUnitHeight, float holeRadiusRatio, int smoothingHeightDiff) {
	m_width = width;
	m_height = height;
	m_tileSize = tileSize;
	m_tileUnitHeight = tileUnitHeight;
	m_holeRadiusRatio = holeRadiusRatio;
	m_smoothingHeightDiff = smoothingHeightDiff;
	m_grid.assign(height, std::vector<TileData>(width, TileData{}));
	m_initialEntities.clear();
	m_holeCords.clear();
	m_modelId = std::nullopt;
}

void map::Map::setTile(int x, int y, const TileData &tile) {
	if (!isValidGrid(x, y))
		return;

	m_grid[y][x] = tile;
}

const map::TileData& map::Map::getTile(int x, int y) const {
	static const TileData defaultTile{};
	if (!isValidGrid(x, y))
		return defaultTile;

	return m_grid[y][x];
}

const map::TileData& map::Map::getTile(CellCord cord) const {
	return getTile(cord.x, cord.y);
}

bool map::Map::isValidGrid(int x, int y) const {
	return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

bool map::Map::isValidGrid(CellCord cord) const {
	return isValidGrid(cord.x, cord.y);
}

map::CellCord map::Map::worldToGrid(Vector3 worldCords) const {
	if (m_width <= 0 || m_height <= 0 || m_tileSize <= constants::epsilon)
		throw std::runtime_error("Map not initialized");

	const float halfWidth = (m_width * m_tileSize) * 0.5f;
	const float halfHeight = (m_height * m_tileSize) * 0.5f;

	const int x = static_cast<int>(std::floor((worldCords.x + halfWidth) / m_tileSize));
	const int y = static_cast<int>(std::floor((worldCords.z + halfHeight) / m_tileSize));

	return CellCord{x, y};
}

const map::TileData& map::Map::getGridData(Vector3 worldCords) const {
	const CellCord cord = worldToGrid(worldCords);
	return getTile(cord);
}

Vector3 map::Map::gridToWorld(int x, int y) const {
	const float halfWidth = (m_width * m_tileSize) * 0.5f;
	const float halfHeight = (m_height * m_tileSize) * 0.5f;

	const float worldX = (x + 0.5f) * m_tileSize - halfWidth;
	const float worldZ = (y + 0.5f) * m_tileSize - halfHeight;
	const float worldY = getTile(x, y).height * m_tileUnitHeight;

	return Vector3{worldX, worldY, worldZ};
}

Vector3 map::Map::gridToWorld(CellCord cellCords) const {
	return gridToWorld(cellCords.x, cellCords.y);
}

bool map::Map::isWithinHole(Vector3 worldPos) const {
	const float holeRadius = getHoleRadius();
	for (const auto &hole : m_holeCords) {
		const Vector3 holeCenter = gridToWorld(hole);
		if (Vector3Distance(worldPos, holeCenter) < holeRadius) {
			return true;
		}
	}
	return false;
}

std::optional<map::CellCord> map::Map::getHoleCellIfInHole(Vector3 worldPos) const {
	const float holeRadius = getHoleRadius();
	for (const auto &hole : m_holeCords) {
		const Vector3 holeCenter = gridToWorld(hole);
		if (Vector3Distance(worldPos, holeCenter) < holeRadius) {
			return hole;
		}
	}
	return std::nullopt;
}

Vector3 map::Map::getHoleCenter(CellCord cell) const {
	return gridToWorld(cell);
}

float map::Map::computeCornerForTile(int tx, int ty, int cx, int cz) const {
	const int selfH = m_grid[ty][tx].height;
	float totalH = 0.0f;
	int count = 0;

	const int dx[4] = {-1, 0, -1, 0};
	const int dy[4] = {-1, -1, 0, 0};

	for (int i = 0; i < 4; ++i) {
		const int sx = cx + dx[i];
		const int sy = cz + dy[i];
		if (!isValidGrid(sx, sy))
			continue;

		const int nH = m_grid[sy][sx].height;
		if (std::abs(nH - selfH) < m_smoothingHeightDiff) {
			totalH += static_cast<float>(nH);
			count++;
		}
	}

	if (count == 0)
		return static_cast<float>(selfH) * m_tileUnitHeight;

	return (totalH / static_cast<float>(count)) * m_tileUnitHeight;
}

float map::Map::getGroundY(Vector3 worldPos, Vector3 *outNormal) const {
	if (m_width <= 0 || m_height <= 0) {
		if (outNormal) *outNormal = utils::math::getUpVector();
		return 0.0f;
	}

	const CellCord cord = worldToGrid(worldPos);
	if (!isValidGrid(cord)) {
		if (outNormal) *outNormal = utils::math::getUpVector();
		return 0.0f;
	}

	const TileData &tile = getTile(cord);
	const float dx = worldPos.x - tile.xMid;
	const float dz = worldPos.z - tile.zMid;

	Vector3 n = utils::math::getUpVector();
	Vector3 p0 = {tile.x1, tile.cornerY.x0z0, tile.z1};

	// formula visualisation: https://www.desmos.com/calculator/4wdhihismu
	if (std::abs(dx) <= -dz) {
		n = tile.normals.z0;
		p0 = {tile.x1, tile.cornerY.x0z0, tile.z1};
	} else if (std::abs(dx) <= dz) {
		n = tile.normals.z1;
		p0 = {tile.x2, tile.cornerY.x1z1, tile.z2};
	} else if (std::abs(dz) <= -dx) {
		n = tile.normals.x0;
		p0 = {tile.x1, tile.cornerY.x0z1, tile.z2};
	} else {
		n = tile.normals.x1;
		p0 = {tile.x2, tile.cornerY.x1z0, tile.z1};
	}

	if (outNormal)
		*outNormal = n;

	if (std::abs(n.y) > constants::epsilon) {
		return p0.y - (n.x * (worldPos.x - p0.x) + n.z * (worldPos.z - p0.z)) / n.y;
	}

	return tile.selfHeight;
}

void map::Map::populateTileBounds() {
	if (m_width <= 0 || m_height <= 0)
		return;

	const float halfWidth = (m_width * m_tileSize) * 0.5f;
	const float halfHeight = (m_height * m_tileSize) * 0.5f;
	const float baseBottom = -2.0f * m_tileUnitHeight;

	for (int y = 0; y < m_height; ++y) {
		for (int x = 0; x < m_width; ++x) {
			TileData &tile = m_grid[y][x];
			tile.cord = CellCord{x, y};
			tile.x1 = x * m_tileSize - halfWidth;
			tile.x2 = (x + 1) * m_tileSize - halfWidth;
			tile.z1 = y * m_tileSize - halfHeight;
			tile.z2 = (y + 1) * m_tileSize - halfHeight;

			tile.xMid = (tile.x1 + tile.x2) * 0.5f;
			tile.zMid = (tile.z1 + tile.z2) * 0.5f;

			tile.selfHeight = static_cast<float>(tile.height) * m_tileUnitHeight;
			tile.upHeight = (y > 0) ? (static_cast<float>(m_grid[y - 1][x].height) * m_tileUnitHeight) : baseBottom;
			tile.downHeight = (y + 1 < m_height) ? (static_cast<float>(m_grid[y + 1][x].height) * m_tileUnitHeight) : baseBottom;
			tile.leftHeight = (x > 0) ? (static_cast<float>(m_grid[y][x - 1].height) * m_tileUnitHeight) : baseBottom;
			tile.rightHeight = (x + 1 < m_width) ? (static_cast<float>(m_grid[y][x + 1].height) * m_tileUnitHeight) : baseBottom;

			tile.cornerY.x0z0 = computeCornerForTile(x, y, x, y);
			tile.cornerY.x1z0 = computeCornerForTile(x, y, x + 1, y);
			tile.cornerY.x0z1 = computeCornerForTile(x, y, x, y + 1);
			tile.cornerY.x1z1 = computeCornerForTile(x, y, x + 1, y + 1);

			tile.yMid = (tile.cornerY.x0z0 + tile.cornerY.x1z0 + tile.cornerY.x0z1 + tile.cornerY.x1z1) * 0.25f;
			const Vector3 c = {tile.xMid, tile.yMid, tile.zMid};

			const Vector3 v00 = {tile.x1, tile.cornerY.x0z0, tile.z1};
			const Vector3 v10 = {tile.x2, tile.cornerY.x1z0, tile.z1};
			const Vector3 v01 = {tile.x1, tile.cornerY.x0z1, tile.z2};
			const Vector3 v11 = {tile.x2, tile.cornerY.x1z1, tile.z2};

			tile.normals.z0 = Vector3Normalize(Vector3CrossProduct(c - v00, v10 - v00));
			tile.normals.z1 = Vector3Normalize(Vector3CrossProduct(c - v11, v01 - v11));
			tile.normals.x0 = Vector3Normalize(Vector3CrossProduct(c - v01, v00 - v01));
			tile.normals.x1 = Vector3Normalize(Vector3CrossProduct(c - v10, v11 - v10));
		}
	}
}

void map::Map::addInitialEntity(const InitialEntity &entity) {
	m_initialEntities.push_back(entity);
}

void map::Map::spawnAll(GameContext &context) const {
	Vector3 playerSpawnPos = {0.0f, 1.0f, 0.0f};
	bool playerSpawned = false;

	for (const auto &entity : m_initialEntities) {
		const Vector3 worldPos = gridToWorld(entity.cellCords) + Vector3{0.0f, 1.0f, 0.0f};
		const Color entityColor = (entity.type == EntityType::Guli) ? WHITE : getRaylibColor(entity.color);

		if (entity.type == EntityType::Player) {
			entity::spawnPlayer(context, worldPos);
			playerSpawnPos = worldPos;
			playerSpawned = true;
		} else if (entity.type == EntityType::Guli) {
			entity::spawnGuli(context, worldPos, 1.0f);
		} else {
			entity::spawnOrb(context, worldPos, entityColor, 1.0f);
		}
	}

	if (!playerSpawned) {
		entity::spawnPlayer(context, playerSpawnPos);
	}

	context.mainCamera.position = playerSpawnPos + Vector3{0.0f, 8.0f, 16.0f};
	context.mainCamera.target = playerSpawnPos;
	context.mainCamera.up = Vector3{0.0f, 1.0f, 0.0f};
	context.mainCamera.fovy = 45.0f;
	context.mainCamera.projection = CAMERA_PERSPECTIVE;
}

void map::Map::clear() {
	m_width = 0;
	m_height = 0;
	m_grid.clear();
	m_initialEntities.clear();
	m_holeCords.clear();
	m_modelId = std::nullopt;
}
