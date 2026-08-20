#include "systems/GuliInHoleSystem.hpp"
#include "components.hpp"
#include "entities.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include <unordered_map>
#include <vector>

using namespace component;

namespace {
	void removePhysicsComponents(entt::registry &registry, entt::entity entity) {
		utils::entt::removeComponents<Velocity, CollisionBody, VerticalGravity, AttractedBy, tags::zeroGravity>(registry, entity);
	}

	void captureGuliAndParticlesInHoles(GameContext &context) {
		for (auto [guliEnt, guliPos] : context.registry.view<Position, tags::GuliTarget>(entt::exclude<MergingInHole>).each()) {
			const auto gHoleCell = context.map.getHoleCellIfInHole(guliPos.value);
			if (!gHoleCell.has_value())
				continue;

			const Vector3 holeCenter = context.map.getHoleCenter(*gHoleCell);
			const Vector3 targetPos = Vector3Add(holeCenter, Vector3{0.0f, context.map.getHoleRadius(), 0.0f});

			removePhysicsComponents(context.registry, guliEnt);
			context.registry.emplace<MergingInHole>(guliEnt, MergingInHole{guliEnt, targetPos});
		}

		for (auto [guliEnt, guliPos, merging] : context.registry.view<Position, MergingInHole, tags::GuliTarget>().each()) {
			const float absorbRadius = context.map.getHoleRadius() * 3.0f;
			for (auto [pEnt, pPos, shotGuli] : context.registry.view<Position, ShotGuli>(entt::exclude<MergingInHole>).each()) {
				(void)shotGuli;
				if (Vector3Distance(pPos.value, guliPos.value) < absorbRadius ||
					Vector3Distance(pPos.value, merging.targetPos) < absorbRadius) {
					removePhysicsComponents(context.registry, pEnt);
					context.registry.emplace<MergingInHole>(pEnt, MergingInHole{guliEnt, merging.targetPos});
				}
			}
		}
	}

	void processMergingParticles(GameContext &context, float dt) {
		constexpr float MERGE_SPEED = 100.0f;
		const float maxRadius = std::numeric_limits<float>::infinity();
		const float step = MERGE_SPEED * dt;

		for (auto [entity, pos, merging] : context.registry.view<Position, MergingInHole>().each()) {
			const float dist = Vector3Distance(pos.value, merging.targetPos);
			if (dist <= step || dist <= constants::epsilon) {
				pos.value = merging.targetPos;
				continue;
			}

			const Vector3 velChange = utils::algorithm::calculateVortexAttractionVelocity(
				pos.value,
				merging.targetPos,
				Vector3{0.0f, 1.0f, 0.0f},
				MERGE_SPEED,
				maxRadius,
				0.2f
			);
			pos.value = Vector3Add(pos.value, Vector3Scale(velChange, dt));
		}
	}

	struct MergeGroup {
		Vector3 targetPos = {0.0f, 0.0f, 0.0f};
		std::vector<entt::entity> entities;
		bool allArrived = true;
	};

	std::unordered_map<entt::entity, MergeGroup> collectMergeGroups(const entt::registry &registry) {
		std::unordered_map<entt::entity, MergeGroup> groups;
		constexpr float ARRIVAL_THRESHOLD = 0.15f;

		for (auto [entity, pos, merging] : registry.view<Position, MergingInHole>().each()) {
			auto &group = groups[merging.guliEntity];
			group.targetPos = merging.targetPos;
			group.entities.push_back(entity);

			const float dist = Vector3Distance(pos.value, merging.targetPos);
			if (dist > ARRIVAL_THRESHOLD) {
				group.allArrived = false;
			}
		}
		return groups;
	}

	Color calculateGroupAverageColor(const entt::registry &registry, const std::vector<entt::entity> &entities) {
		int totalR = 0;
		int totalG = 0;
		int totalB = 0;
		int count = 0;

		for (entt::entity member : entities) {
			const auto *rb = registry.try_get<RenderBody>(member);
			if (!rb)
				continue;

			totalR += rb->color.r;
			totalG += rb->color.g;
			totalB += rb->color.b;
			count++;
		}

		if (count == 0)
			return WHITE;

		return Color{
			static_cast<unsigned char>(totalR / count),
			static_cast<unsigned char>(totalG / count),
			static_cast<unsigned char>(totalB / count),
			255
		};
	}

	void finalizeGroupMerge(GameContext &context, const MergeGroup &group) {
		const Color mergedColor = calculateGroupAverageColor(context.registry, group.entities);
		entt::entity newGuliEnt = entity::spawnOrb(context, group.targetPos, mergedColor, 1.0f);
		context.registry.emplace<tags::CollectibleGuli>(newGuliEnt);
		context.registry.emplace<tags::Glass>(newGuliEnt);
		
		for (entt::entity member : group.entities) {
			context.registry.destroy(member);
		}
	}

	void processGroupMerging(GameContext &context) {
		const auto groups = collectMergeGroups(context.registry);

		for (const auto &[guliEnt, group] : groups) {
			if (!group.allArrived || group.entities.empty())
				continue;

			finalizeGroupMerge(context, group);
		}
	}
}

void systems::GuliInHoleSystem::update(GameContext &context, float dt) {
	captureGuliAndParticlesInHoles(context);
	processMergingParticles(context, dt);
	processGroupMerging(context);
}
