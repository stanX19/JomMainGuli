#pragma once

#include "includes.hpp"
#include <cstdint>

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
		Golem,
		Flag
	};

	struct TileData {
		int height = 0;
		ColorType color = ColorType::None;
	};

	struct InitialEntity {
		Vector2 cellCords = {0.0f, 0.0f};
		EntityType type = EntityType::Slime;
		ColorType color = ColorType::None;
	};
} // namespace map
