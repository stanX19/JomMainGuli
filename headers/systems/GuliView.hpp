#pragma once

#include "includes.hpp"
#include "systems/BaseSystem.hpp"

namespace systems {
	class GuliView : public BaseSystem {
	public:
		GuliView() = default;
		~GuliView() override = default;

		void update(GameContext &context, float dt) override;
	};
} // namespace systems
