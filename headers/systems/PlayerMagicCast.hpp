#pragma once

#include "BaseSystem.hpp"
#include <random>

namespace systems {
	class PlayerMagicCast : public BaseSystem {
	public:
		PlayerMagicCast(): m_rng(std::random_device{}()), m_uDist(0.0f, 1.0f) {}
		PlayerMagicCast(std::mt19937& rng): m_rng(rng), m_uDist(0.0f, 1.0f) {}

		void update(GameContext &context, float dt) override;
		void handleLMBDown(GameContext &context, const Ray &ray, const Vector3 &playerPos, float dt);
		std::optional<Color> sampleTileColor(const map::Map &map, const Vector3 &samplePos, float searchRadius);
	private:
		std::mt19937 m_rng;
		std::uniform_real_distribution<float> m_uDist;
	};
} // namespace systems
