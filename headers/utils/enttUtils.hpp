#pragma once

#include "includes.hpp"

namespace utils::entt {
	template <typename... Components>
	void removeComponents(::entt::registry &registry, ::entt::entity entity) {
		((registry.all_of<Components>(entity) ? (void)registry.remove<Components>(entity) : void()), ...);
	}
} // namespace utils::entt
