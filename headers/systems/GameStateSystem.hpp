#pragma once

#include "systems/BaseSystem.hpp"

namespace systems {
	class GameStateSystem : public BaseSystem {
	public:
		GameStateSystem() = default;
		~GameStateSystem() override = default;

		void update(GameContext &context, float dt) override;
	};
} // namespace systems
