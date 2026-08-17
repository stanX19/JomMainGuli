#include "GameConfig.hpp"
#include "SubGameConfig.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

SubGameConfig GameConfig::getSubConfig(const std::string& path) const {
	return SubGameConfig(this, path);
}

void GameConfig::init(std::initializer_list<RootSource> sources) {
	init(std::vector<RootSource>(sources));
}

void GameConfig::init(const std::vector<RootSource>& sources) {
	if (m_loaded || sources.empty())
		return;

	nlohmann::json candidate = nlohmann::json::object();
	std::map<std::string, RootJsonFile> candidateFiles;

	for (const auto& [rootName, sourcePath] : sources) {
		if (rootName.empty() || sourcePath.empty())
			continue;
		if (!candidateFiles.emplace(rootName, RootJsonFile{sourcePath, false}).second)
			continue;

		std::ifstream file(sourcePath);
		if (!file)
			continue;
		try {
			file >> candidate[rootName];
		} catch (const nlohmann::json::parse_error&) {
			continue;
		}
	}

	m_config = std::move(candidate);
	m_roots = std::move(candidateFiles);
	m_loaded = true;
	initConstants();
}

void GameConfig::initConstants() {
	ARENA_SIZE = getFloat("game.arenaSize", 2000.0f);

	physics.collisionElasticity = getFloat("physics.collisionElasticity", 0.5f);
	physics.maxAngularKick = getFloat("physics.maxAngularKick", 0.5f);
	physics.roughness = getFloat("physics.roughness", 2.5f);

	settings.masterVolume = getFloat("audio.masterVolume", 0.5f);
	settings.controlSensitivity = Clamp(
		getFloat("settings.controlSensitivity", 1.0f), 0.01f, 1.0f
	);

	debug.showTarget = getBool("debug.showTarget", false);
}

const nlohmann::json* GameConfig::navigatePath(const std::string& path) const {
	if (!m_loaded)
		return nullptr;

	const nlohmann::json* current = &m_config;
	std::istringstream stream(path);
	std::string token;
	while (std::getline(stream, token, '.')) {
		if (token.empty() || !current->is_object() || !current->contains(token))
			return nullptr;
		current = &(*current)[token];
	}
	return current;
}

nlohmann::json* GameConfig::navigatePath(
	nlohmann::json& root,
	const std::string& path
) const {
	if (path.empty())
		return nullptr;

	nlohmann::json* current = &root;
	std::istringstream stream(path);
	std::string token;
	while (std::getline(stream, token, '.')) {
		if (token.empty())
			return nullptr;
		if (!current->is_object())
			*current = nlohmann::json::object();
		current = &(*current)[token];
	}
	return current;
}

float GameConfig::getFloat(const std::string& path, float defaultVal) const {
	const nlohmann::json* node = navigatePath(path);
	if (node == nullptr || !node->is_number())
		return defaultVal;
	return node->get<float>();
}

int GameConfig::getInt(const std::string& path, int defaultVal) const {
	const nlohmann::json* node = navigatePath(path);
	if (node == nullptr || !node->is_number())
		return defaultVal;
	return node->get<int>();
}

bool GameConfig::getBool(const std::string& path, bool defaultVal) const {
	const nlohmann::json* node = navigatePath(path);
	if (node == nullptr || !node->is_boolean())
		return defaultVal;
	return node->get<bool>();
}

std::string GameConfig::getString(
	const std::string& path,
	const std::string& defaultVal
) const {
	const nlohmann::json* node = navigatePath(path);
	if (node == nullptr || !node->is_string())
		return defaultVal;
	return node->get<std::string>();
}

std::vector<std::string> GameConfig::getStringArray(
	const std::string& path,
	const std::vector<std::string>& defaultVal
) const {
	const nlohmann::json* node = navigatePath(path);
	if (node == nullptr || !node->is_array())
		return defaultVal;
	std::vector<std::string> result;
	result.reserve(node->size());
	for (const auto& value : *node) {
		if (!value.is_string())
			return defaultVal;
		result.push_back(value.get<std::string>());
	}
	return result;
}

Vector3 GameConfig::getVector3(
	const std::string& path,
	Vector3 defaultVal
) const {
	const nlohmann::json* node = navigatePath(path);
	if (node == nullptr || !node->is_object())
		return defaultVal;
	return Vector3{
		node->value("x", defaultVal.x),
		node->value("y", defaultVal.y),
		node->value("z", defaultVal.z)
	};
}

nlohmann::json GameConfig::getSection(const std::string& path) const {
	const nlohmann::json* node = navigatePath(path);
	return node == nullptr ? nlohmann::json{} : *node;
}

void GameConfig::setJsonValue(const std::string& path, nlohmann::json value) {
	const std::size_t separator = path.find('.');
	if (separator == std::string::npos || separator == 0 || separator + 1 >= path.size())
		return;
	const std::string rootName = path.substr(0, separator);
	const auto root = m_roots.find(rootName);
	if (root == m_roots.end())
		return;

	nlohmann::json updatedRoot = m_config.at(rootName);
	nlohmann::json* node = navigatePath(
		updatedRoot,
		path.substr(separator + 1)
	);
	if (node == nullptr || *node == value)
		return;
	*node = std::move(value);

	m_config[rootName] = std::move(updatedRoot);
	root->second.dirty = true;
	initConstants();
}

void GameConfig::setFloat(const std::string& path, float value) {
	setJsonValue(path, value);
}

void GameConfig::setString(const std::string& path, const std::string& value) {
	setJsonValue(path, value);
}

void GameConfig::setStringArray(
	const std::string& path,
	const std::vector<std::string>& value
) {
	nlohmann::json array = nlohmann::json::array();
	for (const std::string& entry : value)
		array.push_back(entry);
	setJsonValue(path, std::move(array));
}

void GameConfig::setBool(const std::string& path, bool value) {
	setJsonValue(path, value);
}

void GameConfig::saveRootJsonFile(
	const std::string& rootName,
	RootJsonFile& file
) {
	std::ofstream output(file.sourcePath, std::ios::trunc);
	if (!output)
		return;
	output << m_config.at(rootName).dump(4) << '\n';
	file.dirty = false;
}

void GameConfig::saveRoot(const std::string& rootName) {
	const auto iterator = m_roots.find(rootName);
	if (iterator != m_roots.end())
		saveRootJsonFile(rootName, iterator->second);
}

void GameConfig::saveChanged() {
	for (auto& [rootName, file] : m_roots) {
		if (file.dirty)
			saveRootJsonFile(rootName, file);
	}
}

void GameConfig::saveAll() {
	for (auto& [rootName, file] : m_roots)
		saveRootJsonFile(rootName, file);
}
