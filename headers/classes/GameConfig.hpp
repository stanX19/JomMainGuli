#pragma once

#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "json.hpp"
#include "raylib.h"

class SubGameConfig;

class GameConfig {
public:
	using RootSource = std::pair<std::string, std::string>;

	GameConfig();
	explicit GameConfig(const std::string& configDir);
	virtual ~GameConfig() = default;

	void init(const std::string& configDir = "assets/config");
	void init(const std::vector<RootSource>& sources);
	void init(std::initializer_list<RootSource> sources);
	void initConstants();
	bool isLoaded() const { return m_loaded; }

	virtual float getFloat(const std::string& path, float defaultVal) const;
	virtual int getInt(const std::string& path, int defaultVal) const;
	virtual bool getBool(const std::string& path, bool defaultVal) const;
	virtual std::string getString(
		const std::string& path,
		const std::string& defaultVal
	) const;
	virtual std::vector<std::string> getStringArray(
		const std::string& path,
		const std::vector<std::string>& defaultVal
	) const;
	virtual Vector3 getVector3(const std::string& path, Vector3 defaultVal) const;

	void setFloat(const std::string& path, float value);
	void setBool(const std::string& path, bool value);
	void setString(const std::string& path, const std::string& value);
	void setStringArray(
		const std::string& path,
		const std::vector<std::string>& value
	);

	void saveRoot(const std::string& rootName);
	void saveChanged();
	void saveAll();

	const nlohmann::json& getJson() const { return m_config; }
	nlohmann::json getSection(const std::string& path) const;
	SubGameConfig getSubConfig(const std::string& path) const;

	struct Physics {
		float collisionElasticity = 0.5f;
		float maxAngularKick = 0.5f;
		float roughness = 2.5f;
		float gravity = 120.0f;
	} physics;

	struct Magic {
		float spellCastRadius = 50.0f;
		float focusDistance = 10.0f;
		float shootSpeed = 120.0f;
		float spawnInterval = 0.15f;
		int maxSpawnCount = 15;
		float gravitonLifespan = 10.0f;
	} magic;

	struct Map {
		float tileSize = 50.0f;
		float tileUnitHeight = 20.0f;
		float holeRadiusRatio = 0.6f;
	} map;

	struct Settings {
		float masterVolume = 0.5f;
		float controlSensitivity = 1.0f;
	} settings;

	struct Debug {
		bool showTarget = false;
	} debug;

	float ARENA_SIZE = 2000.0f;

private:
	struct RootJsonFile {
		std::string sourcePath;
		bool dirty = false;
	};

	nlohmann::json m_config;
	std::map<std::string, RootJsonFile> m_roots;
	bool m_loaded = false;

	const nlohmann::json* navigatePath(const std::string& path) const;
	nlohmann::json* navigatePath(
		nlohmann::json& root,
		const std::string& path
	) const;
	void setJsonValue(const std::string& path, nlohmann::json value);
	void saveRootJsonFile(const std::string& rootName, RootJsonFile& file);
};
