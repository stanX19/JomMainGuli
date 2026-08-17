#pragma once

#include "includes.hpp"
#include "map/Map.hpp"
#include <string>
#include <vector>
#include <set>
#include <optional>

namespace {
	// for char map tiles
	struct CharColorDef {
		char symbol;
		map::ColorType type;
	};

	// for char map entities
	struct CharEntityDef {
		char symbol;
		map::EntityType type;
	};
}

namespace map {
	class MapLoader {
	public:
		static Map load(const std::string &mapPath);

	private:
		static constexpr CharColorDef COLOR_DEFS[] = {
			{'R', ColorType::Red},
			{'r', ColorType::Red},
			{'G', ColorType::Green},
			{'g', ColorType::Green},
			{'B', ColorType::Blue},
			{'b', ColorType::Blue},
			{'Y', ColorType::Yellow},
			{'y', ColorType::Yellow},
			{'P', ColorType::Purple},
			{'p', ColorType::Purple},
			{'O', ColorType::Orange},
			{'o', ColorType::Orange},
			{'W', ColorType::White},
			{'w', ColorType::White},
		};

		static constexpr CharEntityDef ENTITY_DEFS[] = {
			{'P', EntityType::Player},
			{'S', EntityType::Slime},
			{'G', EntityType::Golem},
			{'M', EntityType::Flag},
		};

		static ColorType parseColorChar(char c);
		static std::optional<EntityType> parseEntityChar(char c);
		static int parseExplicitHeight(char c);
		static std::string deriveColorPath(const std::string &mapPath);
		static void validate(
			const std::vector<std::string> &mapLines,
			const std::vector<std::string> &colorLines
		);
		static int calculateMinNeighborHeight(
			const std::vector<std::vector<int>> &heightGrid,
			const std::vector<std::vector<bool>> &isEntityGrid,
			int x, int y, int width, int height
		);
		static std::vector<std::vector<int>> buildHeightGrid(
			const std::vector<std::string> &mapLines
		);
		static std::vector<InitialEntity> extractInitialEntities(
			const std::vector<std::string> &mapLines,
			const std::vector<std::string> &colorLines,
			Vector2 &outFlagCords
		);
		static void populateMap(
			Map &map,
			const std::vector<std::vector<int>> &heightGrid,
			const std::vector<std::string> &colorLines,
			const std::vector<InitialEntity> &entities,
			Vector2 flagCords
		);
	};
} // namespace map
