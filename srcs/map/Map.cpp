#include "map/Map.hpp"
#include "GameContext.hpp"
#include "entities.hpp"
#include <cmath>

Color map::Map::toRaylibColor(ColorType type) {
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
			return LIGHTGRAY;
	}
}

map::Map::Map() = default;

void map::Map::init(int width, int height, float tileSize, float heightScale) {
	m_width = width;
	m_height = height;
	m_tileSize = tileSize;
	m_heightScale = heightScale;
	m_grid.assign(height, std::vector<TileData>(width, TileData{}));
	m_initialEntities.clear();
	m_flagCords = Vector2{0.0f, 0.0f};
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

bool map::Map::isValidGrid(int x, int y) const {
	return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

const map::TileData& map::Map::getGridData(Vector3 worldCords) const {
	static const TileData defaultTile{};
	if (m_width <= 0 || m_height <= 0 || m_tileSize <= 0.0001f)
		return defaultTile;

	const float halfWidth = (m_width * m_tileSize) * 0.5f;
	const float halfHeight = (m_height * m_tileSize) * 0.5f;

	const int x = static_cast<int>(std::floor((worldCords.x + halfWidth) / m_tileSize));
	const int y = static_cast<int>(std::floor((worldCords.z + halfHeight) / m_tileSize));

	if (!isValidGrid(x, y))
		return defaultTile;

	return m_grid[y][x];
}

Vector3 map::Map::gridToWorld(int x, int y) const {
	const float halfWidth = (m_width * m_tileSize) * 0.5f;
	const float halfHeight = (m_height * m_tileSize) * 0.5f;

	const float worldX = (x + 0.5f) * m_tileSize - halfWidth;
	const float worldZ = (y + 0.5f) * m_tileSize - halfHeight;
	const float worldY = getTile(x, y).height * m_heightScale;

	return Vector3{worldX, worldY, worldZ};
}

Vector3 map::Map::gridToWorld(Vector2 cellCords) const {
	return gridToWorld(static_cast<int>(cellCords.x), static_cast<int>(cellCords.y));
}

void map::Map::addInitialEntity(const InitialEntity &entity) {
	m_initialEntities.push_back(entity);
}

void map::Map::spawnAll(GameContext &context) const {
	Vector3 playerSpawnPos = {0.0f, 1.0f, 0.0f};
	bool playerSpawned = false;

	for (const auto &entity : m_initialEntities) {
		const int x = static_cast<int>(entity.cellCords.x);
		const int y = static_cast<int>(entity.cellCords.y);
		const Vector3 worldPos = gridToWorld(x, y) + Vector3{0.0f, 1.0f, 0.0f};
		const Color entityColor = toRaylibColor(entity.color);

		if (entity.type == EntityType::Player) {
			entity::spawnPlayer(context, worldPos);
			playerSpawnPos = worldPos;
			playerSpawned = true;
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
	m_flagCords = Vector2{0.0f, 0.0f};
	m_modelId = std::nullopt;
}
