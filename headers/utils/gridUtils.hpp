#pragma once

#include <vector>
#include <string>

namespace utils::grid {
	template <typename Func, typename T = int>
	T sum(const std::vector<std::string> &grid, Func &&func, T initialValue = 0) {
		T total = initialValue;
		for (const auto &line : grid) {
			for (char c : line) {
				total += func(c);
			}
		}
		return total;
	}

	template <typename Predicate>
	int countIf(const std::vector<std::string> &grid, Predicate &&pred) {
		return sum(grid, [&](char c) { return pred(c) ? 1 : 0; });
	}

	template <typename Predicate>
	int anyOf(const std::vector<std::string> &grid, Predicate &&pred) {
		return countIf(grid, pred) > 0;
	}

	template <typename Predicate>
	bool allOf(const std::vector<std::string> &grid, Predicate &&pred) {
		for (const auto &line : grid) {
			for (char c : line) {
				if (!pred(c))
					return false;
			}
		}
		return true;
	}
} // namespace utils::grid