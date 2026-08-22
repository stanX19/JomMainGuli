#include "entities.hpp"
#include "components.hpp"
#include "map/Map.hpp"
#include <set>
#include <vector>
#include <string>
#include <optional>

using namespace component;

namespace {
	struct ColorCompare {
		bool operator()(const Color &a, const Color &b) const {
			if (a.r != b.r) return a.r < b.r;
			if (a.g != b.g) return a.g < b.g;
			if (a.b != b.b) return a.b < b.b;
			return a.a < b.a;
		}
	};

	std::vector<Color> deduplicateColors(const std::vector<Color> &colors) {
		std::set<Color, ColorCompare> uniqueSet(colors.begin(), colors.end());
		return std::vector<Color>(uniqueSet.begin(), uniqueSet.end());
	}

	struct HeritageRecipe {
		std::string name;
		std::string imagePath;
	};

	bool hasColor(const std::vector<Color> &colors, map::ColorType colorType) {
		for (const Color &c : colors) {
			if (map::Map::getColorType(c) == colorType)
				return true;
		}
		return false;
	}

	std::optional<HeritageRecipe> evaluateHeritageRecipe(const std::vector<Color> &colors) {
		bool hasRed = hasColor(colors, map::ColorType::Red);
		bool hasGreen = hasColor(colors, map::ColorType::Green);
		bool hasBlue = hasColor(colors, map::ColorType::Blue);
		bool hasYellow = hasColor(colors, map::ColorType::Yellow);
		bool hasWhite = hasColor(colors, map::ColorType::White);

		if (hasRed && hasBlue && hasYellow) {
			return HeritageRecipe{"Jalur Gemilang", "assets/images/malaysia_flag.png"};
		}
		if (hasGreen && hasRed && !hasBlue) {
			return HeritageRecipe{"Nasi Lemak", "assets/images/nasi_lemak.png"};
		}
		if (hasRed && hasYellow && !hasBlue) {
			return HeritageRecipe{"Bunga Raya", "assets/images/bunga_raya.png"};
		}
		if (hasBlue && (hasYellow || hasWhite) && !hasRed && !hasGreen) {
			return HeritageRecipe{"Petronas Twin Towers", "assets/images/petronas_tower.png"};
		}

		return std::nullopt;
	}
} // namespace

entt::entity entity::spawnGuli(GameContext &context, Vector3 pos, float radius) {
	const entt::entity guli = context.registry.create();

	context.registry.emplace<Position>(guli, Position{pos});
	context.registry.emplace<PrevPosition>(guli, PrevPosition{pos});
	context.registry.emplace<Velocity>(guli, Velocity{Vector3{0.0f, 0.0f, 0.0f}});
	context.registry.emplace<Rotation>(guli, Rotation{QuaternionIdentity()});
	context.registry.emplace<PrevRotation>(guli, PrevRotation{QuaternionIdentity()});
	context.registry.emplace<Mass>(guli, Mass{4.0f});
	context.registry.emplace<VerticalGravity>(guli, VerticalGravity{1.0f});

	const ModelId ribbonModelId = context.modelManager.createRibbon({DARKGREEN, GOLD});
	context.registry.emplace<RenderBody>(guli, RenderBody{ribbonModelId, WHITE, radius});
	context.registry.emplace<CollisionBody>(guli, CollisionBody{radius});
	context.registry.emplace<tags::GuliTarget>(guli);
	context.registry.emplace<tags::IsCoveredByGlass>(guli);
	context.registry.emplace<tags::GlassCollisionSound>(guli);
	context.registry.emplace<tags::RollsOnFloor>(guli);

	return guli;
}

entt::entity entity::spawnCollectibleGuli(GameContext &context, Vector3 pos, const std::vector<Color> &colors, float radius) {
	const entt::entity collectibleGuli = context.registry.create();

	context.registry.emplace<Position>(collectibleGuli, Position{pos});
	context.registry.emplace<PrevPosition>(collectibleGuli, PrevPosition{pos});
	context.registry.emplace<Velocity>(collectibleGuli, Velocity{Vector3{0.0f, 0.0f, 0.0f}});
	context.registry.emplace<Rotation>(collectibleGuli, Rotation{QuaternionIdentity()});
	context.registry.emplace<PrevRotation>(collectibleGuli, PrevRotation{QuaternionIdentity()});
	context.registry.emplace<Mass>(collectibleGuli, Mass{4.0f});
	context.registry.emplace<VerticalGravity>(collectibleGuli, VerticalGravity{0.0f});
	context.registry.emplace<tags::zeroGravity>(collectibleGuli);

	const std::vector<Color> uniqueColors = deduplicateColors(colors);
	const auto heritageOpt = evaluateHeritageRecipe(uniqueColors);

	ModelId innerModelId = 0;
	if (heritageOpt.has_value()) {
		innerModelId = context.modelManager.loadModelFromImage(heritageOpt->imagePath, radius * 1.3f);
		context.registry.emplace<tags::IsSpecialGuli>(collectibleGuli);
		context.registry.emplace<Description>(collectibleGuli, Description{heritageOpt->name});
	} else {
		innerModelId = context.modelManager.createRibbon(uniqueColors);
		std::string desc = "Classical Guli";
		if (uniqueColors.size() >= 4) {
			desc = "Super Rare Guli";
		} else if (uniqueColors.size() >= 3) {
			desc = "Colorful Guli";
		}
		context.registry.emplace<Description>(collectibleGuli, Description{desc});
	}

	context.registry.emplace<RenderBody>(collectibleGuli, RenderBody{innerModelId, WHITE, radius});
	context.registry.emplace<CollisionBody>(collectibleGuli, CollisionBody{radius});
	context.registry.emplace<tags::CollectibleGuli>(collectibleGuli);
	context.registry.emplace<tags::IsCoveredByGlass>(collectibleGuli);
	context.registry.emplace<tags::GlassCollisionSound>(collectibleGuli);
	context.registry.emplace<tags::RollsOnFloor>(collectibleGuli);

	return collectibleGuli;

}

entt::entity entity::spawnCollectibleGuli(GameContext &context, const std::vector<Color> &colors) {
	return spawnCollectibleGuli(context, Vector3{0.0f, 0.0f, 0.0f}, colors, 1.0f);
}

