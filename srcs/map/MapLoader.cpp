#include "map/MapLoader.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

namespace {
	std::vector<std::string> readFileLines(const std::string &filePath) {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			return {};
		}

		std::vector<std::string> lines;
		std::string line;
		while (std::getline(file, line)) {
			if (!line.empty() && line.back() == '\r') {
				line.pop_back();
			}
			if (line.empty())
				continue;
			lines.push_back(line);
		}
		return lines;
	}
}

std::string map::MapLoader::deriveColorPath(const std::string &mapPath) {
	const std::size_t dotPos = mapPath.rfind('.');
	if (dotPos == std::string::npos) {
		return mapPath + ".color";
	}
	return mapPath.substr(0, dotPos) + ".color";
}

map::ColorType map::MapLoader::parseColorChar(char c) {
	for (const auto &def : COLOR_DEFS) {
		if (def.symbol == c) {
			return def.type;
		}
	}
	return ColorType::None;
}

std::optional<map::EntityType> map::MapLoader::parseEntityChar(char c) {
	for (const auto &def : ENTITY_DEFS) {
		if (def.symbol == c) {
			return def.type;
		}
	}
	return std::nullopt;
}

int map::MapLoader::parseExplicitHeight(char c) {
	if (c == '#')
		return 11;
	if (c == '0')
		return -1;
	if (c >= '1' && c <= '9')
		return (c - '1');
	return 0;
}

int map::MapLoader::calculateMinNeighborHeight(
	const std::vector<std::vector<int>> &heightGrid,
	const std::vector<std::vector<bool>> &isEntityGrid,
	int x, int y, int width, int height
) {
	int minH = 1000;
	bool found = false;

	const int dx[] = {-1, 1, 0, 0, -1, -1, 1, 1};
	const int dy[] = {0, 0, -1, 1, -1, 1, -1, 1};

	for (int i = 0; i < 8; ++i) {
		const int nx = x + dx[i];
		const int ny = y + dy[i];
		if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
			if (!isEntityGrid[ny][nx]) {
				minH = std::min(minH, heightGrid[ny][nx]);
				found = true;
			}
		}
	}

	return found ? minH : 0;
}

void map::MapLoader::validate(
	const std::vector<std::string> &mapLines,
	const std::vector<std::string> &colorLines
) {
	if (mapLines.empty()) {
		throw std::runtime_error("MapLoader validation failed: Map file is empty");
	}

	const std::size_t width = mapLines[0].size();
	if (width == 0) {
		throw std::runtime_error("MapLoader validation failed: Map width is 0");
	}

	for (std::size_t i = 0; i < mapLines.size(); ++i) {
		if (mapLines[i].size() != width) {
			throw std::runtime_error(
				"MapLoader validation failed: Map is not rectangular (row " +
				std::to_string(i) + " size " + std::to_string(mapLines[i].size()) +
				" != expected " + std::to_string(width) + ")"
			);
		}
	}

	bool hasPlayer = false;
	bool hasFlag = false;

	for (const auto &line : mapLines) {
		for (const char c : line) {
			const auto entityOpt = parseEntityChar(c);
			if (!entityOpt.has_value())
				continue;

			if (*entityOpt == EntityType::Player) {
				hasPlayer = true;
			} else if (*entityOpt == EntityType::Flag) {
				hasFlag = true;
			}
		}
	}

	if (!hasPlayer) {
		throw std::runtime_error("MapLoader validation failed: Map is missing player ('P')");
	}

	if (!hasFlag) {
		throw std::runtime_error("MapLoader validation failed: Map is missing flag ('M')");
	}

	std::set<ColorType> foundColors;
	for (const auto &line : colorLines) {
		for (const char c : line) {
			const ColorType colorType = parseColorChar(c);
			if (colorType != ColorType::None) {
				foundColors.insert(colorType);
			}
		}
	}

	const ColorType requiredColors[] = {
		ColorType::Red,
		ColorType::Green,
		ColorType::Blue,
		ColorType::Yellow
	};

	for (const ColorType rc : requiredColors) {
		if (foundColors.find(rc) == foundColors.end()) {
			throw std::runtime_error("MapLoader validation failed: Map is missing required color (RGBY required)");
		}
	}
}

std::vector<std::vector<int>> map::MapLoader::buildHeightGrid(
	const std::vector<std::string> &mapLines
) {
	const int width = static_cast<int>(mapLines[0].size());
	const int height = static_cast<int>(mapLines.size());

	std::vector<std::vector<int>> heightGrid(height, std::vector<int>(width, 0));
	std::vector<std::vector<bool>> isEntityGrid(height, std::vector<bool>(width, false));

	for (int y = 0; y < height; ++y) {
		const std::string &mLine = mapLines[y];
		for (int x = 0; x < width; ++x) {
			const char mChar = (x < static_cast<int>(mLine.size())) ? mLine[x] : ' ';
			const auto entityOpt = parseEntityChar(mChar);

			if (entityOpt.has_value()) {
				isEntityGrid[y][x] = true;
				heightGrid[y][x] = 0;
			} else {
				heightGrid[y][x] = parseExplicitHeight(mChar);
			}
		}
	}

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			if (isEntityGrid[y][x]) {
				heightGrid[y][x] = calculateMinNeighborHeight(heightGrid, isEntityGrid, x, y, width, height);
			}
		}
	}

	return heightGrid;
}

std::vector<map::InitialEntity> map::MapLoader::extractInitialEntities(
	const std::vector<std::string> &mapLines,
	const std::vector<std::string> &colorLines,
	Vector2 &outFlagCords
) {
	const int width = static_cast<int>(mapLines[0].size());
	const int height = static_cast<int>(mapLines.size());
	std::vector<InitialEntity> entities;

	for (int y = 0; y < height; ++y) {
		const std::string &mLine = mapLines[y];
		for (int x = 0; x < width; ++x) {
			const char mChar = (x < static_cast<int>(mLine.size())) ? mLine[x] : ' ';
			const auto entityOpt = parseEntityChar(mChar);

			if (!entityOpt.has_value())
				continue;

			char cChar = ' ';
			if (y < static_cast<int>(colorLines.size()) && x < static_cast<int>(colorLines[y].size())) {
				cChar = colorLines[y][x];
			}
			const ColorType colorType = parseColorChar(cChar);

			InitialEntity entity{};
			entity.cellCords = Vector2{static_cast<float>(x), static_cast<float>(y)};
			entity.type = *entityOpt;
			entity.color = colorType;
			entities.push_back(entity);

			if (*entityOpt == EntityType::Flag) {
				outFlagCords = entity.cellCords;
			}
		}
	}

	return entities;
}

void map::MapLoader::populateMap(
	Map &map,
	const std::vector<std::vector<int>> &heightGrid,
	const std::vector<std::string> &colorLines,
	const std::vector<InitialEntity> &entities,
	Vector2 flagCords
) {
	const int height = static_cast<int>(heightGrid.size());
	const int width = height > 0 ? static_cast<int>(heightGrid[0].size()) : 0;

	map.init(width, height);
	map.setFlagCords(flagCords);

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			char cChar = ' ';
			if (y < static_cast<int>(colorLines.size()) && x < static_cast<int>(colorLines[y].size())) {
				cChar = colorLines[y][x];
			}
			const ColorType colorType = parseColorChar(cChar);

			TileData tile{};
			tile.height = heightGrid[y][x];
			tile.color = colorType;
			map.setTile(x, y, tile);
		}
	}

	for (const auto &entity : entities) {
		map.addInitialEntity(entity);
	}
}

map::Map map::MapLoader::load(const std::string &mapPath) {
	const std::vector<std::string> mapLines = readFileLines(mapPath);
	if (mapLines.empty()) {
		throw std::runtime_error("MapLoader: Failed to open or read map file '" + mapPath + "'");
	}

	const std::string colorPath = deriveColorPath(mapPath);
	const std::vector<std::string> colorLines = readFileLines(colorPath);

	validate(mapLines, colorLines);

	Vector2 flagCords = {0.0f, 0.0f};
	const auto heightGrid = buildHeightGrid(mapLines);
	const auto entities = extractInitialEntities(mapLines, colorLines, flagCords);

	Map mapResult;
	populateMap(mapResult, heightGrid, colorLines, entities, flagCords);
	return mapResult;
}
