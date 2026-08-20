#pragma once

#include "ModelManager.hpp"

namespace component {
	namespace tags {
		struct IsCoveredByGlass {};
		using isCoveredByGlass = IsCoveredByGlass;
		using Glass = IsCoveredByGlass;
	}

	struct RenderBody {
		ModelId modelID = 0;
		Color color = WHITE;
		Vector3 scale = {1.0f, 1.0f, 1.0f};
		Vector3 translation = {0.0f, 0.0f, 0.0f};
		Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};

		RenderBody() = default;

		RenderBody(ModelId id, float uniformScale)
			: modelID(id), color(WHITE), scale({uniformScale, uniformScale, uniformScale}),
			  translation({0.0f, 0.0f, 0.0f}), rotation({0.0f, 0.0f, 0.0f, 1.0f}) {}

		RenderBody(ModelId id, Color color, float uniformScale,
		           Vector3 translation = {0.0f, 0.0f, 0.0f},
		           Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f})
			: modelID(id), color(color), scale({uniformScale, uniformScale, uniformScale}),
			  translation(translation), rotation(rotation) {}

		RenderBody(ModelId id, Color color, Vector3 scale,
		           Vector3 translation = {0.0f, 0.0f, 0.0f},
		           Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f})
			: modelID(id), color(color), scale(scale),
			  translation(translation), rotation(rotation) {}
	};
} // namespace component
