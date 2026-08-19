#pragma once

#include "BaseSystem.hpp"

namespace systems {
	class GuliInHoleSystem : public BaseSystem {
	public:
		void update(GameContext &context, float dt) override;
	};
} // namespace systems
