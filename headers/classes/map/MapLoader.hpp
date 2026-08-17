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
			{'G', ColorType::Green},
			{'B', ColorType::Blue},
			{'Y', ColorType::Yellow},
			{'P', ColorType::Purple},
			{'O', ColorType::Orange},
			{'W', ColorType::White},
			{' ', ColorType::None},
			{'#', ColorType::None}
		};

		static constexpr CharEntityDef ENTITY_DEFS[] = {
			{'P', EntityType::Player},
			{'S', EntityType::Slime},
			{'G', EntityType::Golem},
			{'M', EntityType::Flag},
		};

		static bool isValidMapChar(char c);
		static bool isValidColorChar(char c);
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
			CellCord &outFlagCords
		);
		static void populateMap(
			Map &map,
			const std::vector<std::vector<int>> &heightGrid,
			const std::vector<std::string> &colorLines,
			const std::vector<InitialEntity> &entities,
			CellCord flagCords
		);
	};
} // namespace map
