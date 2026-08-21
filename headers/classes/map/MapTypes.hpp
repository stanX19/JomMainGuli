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

	struct CornerY {
		float x0z0 = 0.0f;
		float x1z0 = 0.0f;
		float x0z1 = 0.0f;
		float x1z1 = 0.0f;
	};

	struct TriangleNormals {
		Vector3 z0 = {0.0f, 1.0f, 0.0f}; // Triangle at -Z
		Vector3 z1 = {0.0f, 1.0f, 0.0f}; // Triangle at +Z
		Vector3 x0 = {0.0f, 1.0f, 0.0f}; // Triangle at -X
		Vector3 x1 = {0.0f, 1.0f, 0.0f}; // Triangle at +X
	};

	struct TileData {
		CellCord cord = {0, 0};
		int height = 0;
		ColorType color = ColorType::None;

		// world cords
		float x1 = 0.0f;
		float z1 = 0.0f;
		float x2 = 0.0f;
		float z2 = 0.0f;

		// center in world cords
		float xMid = 0.0f;
		float zMid = 0.0f;
		float yMid = 0.0f;

		// neighbors height
		float selfHeight = 0.0f;
		float upHeight = 0.0f;
		float downHeight = 0.0f;
		float leftHeight = 0.0f;
		float rightHeight = 0.0f;

		CornerY cornerY;
		TriangleNormals normals;

		bool operator==(const TileData &other) const {
			return cord == other.cord;
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

