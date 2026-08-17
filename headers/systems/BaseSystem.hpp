#pragma once

#include "components.hpp"
#include "game_context.hpp"

namespace systems {
	class BaseSystem {
	public:
		virtual ~BaseSystem() = default;
		virtual void update(GameContext &context, float dt) = 0;
	};
} // namespace systems
