#pragma once

#include "GameConfig.hpp"
#include <string>
#include <vector>

class SubGameConfig : public GameConfig {
public:
	SubGameConfig(const GameConfig* parent, const std::string& root);
	~SubGameConfig() override = default;

	float getFloat(const std::string& path, float defaultVal) const override;
	int getInt(const std::string& path, int defaultVal) const override;
	bool getBool(const std::string& path, bool defaultVal) const override;
	std::string getString(
		const std::string& path,
		const std::string& defaultVal
	) const override;
	std::vector<std::string> getStringArray(
		const std::string& path,
		const std::vector<std::string>& defaultVal
	) const override;
	Vector3 getVector3(const std::string& path, Vector3 defaultVal) const override;
	Color getColor(const std::string& path, Color defaultVal) const override;

private:
	const GameConfig* m_parentCfg = nullptr;
	std::string m_rootPath;
};
