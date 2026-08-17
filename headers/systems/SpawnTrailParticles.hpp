#pragma once

#include "BaseSystem.hpp"

namespace systems {
	class SpawnTrailParticles : public BaseSystem {
	public:
		void update(GameContext &context, float dt) override;
	};
} // namespace systems
