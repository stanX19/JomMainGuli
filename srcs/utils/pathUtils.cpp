#include "utils.hpp"
#include <filesystem>
#include <string>

std::string utils::path::getParentDir(const std::string &path) {
	return std::filesystem::path(path).parent_path().string();
}

std::string utils::path::getFileName(const std::string &path) {
	return std::filesystem::path(path).filename().string();
}