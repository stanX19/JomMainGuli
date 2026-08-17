#pragma once

#include "BaseSystem.hpp"
#include "components/camera.hpp"

namespace systems {
	class CameraFollowPlayer : public BaseSystem {
	public:
		void update(GameContext &context, float dt) override;

	private:
		component::UnitCamera m_defaultCamera{};
	};
} // namespace systems
