#pragma once

#include "includes.hpp"
#include <cstdint>
#include <cstddef>
#include <functional>

namespace map {
	enum class ColorType : uint8_t {
		None = 0,
		Red,
		Green,
		Blue,
		Yellow,
		Purple,
		Orange,
		White
	};

	enum class EntityType : uint8_t {
		Player,
		Slime,
		Guli,
		Lubang
	};

	struct CellCord {
		int x = 0;
		int y = 0;

		bool operator==(const CellCord &other) const {
			return x == other.x && y == other.y;
		}

		bool operator!=(const CellCord &other) const {
			return !(*this == other);
		}
	};

	struct TileData {
		int height = 0;
		ColorType color = ColorType::None;

		float x1 = 0.0f;
		float z1 = 0.0f;
		float x2 = 0.0f;
		float z2 = 0.0f;

		float selfHeight = 0.0f;
		float upHeight = 0.0f;
		float downHeight = 0.0f;
		float leftHeight = 0.0f;
		float rightHeight = 0.0f;

		bool operator==(const TileData &other) const {
			return (x1 == other.x1 && z1 == other.z1);
		}
	};

	struct InitialEntity {
		CellCord cellCords = {0, 0};
		EntityType type = EntityType::Slime;
		ColorType color = ColorType::None;
	};

	struct TileCollisionData {
		TileData tile;
		Vector3 contactPoint;
	};
} // namespace map

