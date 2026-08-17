#pragma once

#include "BaseSystem.hpp"

namespace systems {
	class DetectEntityCollision : public BaseSystem {
	public:
		void update(GameContext &context, float dt) override;
	};
} // namespace systems
