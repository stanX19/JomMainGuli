#include "SubGameConfig.hpp"

SubGameConfig::SubGameConfig(const GameConfig* parent, const std::string& root)
	: m_parentCfg(parent), m_rootPath(root) {}

float SubGameConfig::getFloat(const std::string& path, float defaultVal) const {
	if (!m_parentCfg) return defaultVal;
	return m_parentCfg->getFloat(m_rootPath + "." + path, defaultVal);
}

int SubGameConfig::getInt(const std::string& path, int defaultVal) const {
	if (!m_parentCfg) return defaultVal;
	return m_parentCfg->getInt(m_rootPath + "." + path, defaultVal);
}

bool SubGameConfig::getBool(const std::string& path, bool defaultVal) const {
	if (!m_parentCfg) return defaultVal;
	return m_parentCfg->getBool(m_rootPath + "." + path, defaultVal);
}

std::string SubGameConfig::getString(
	const std::string& path,
	const std::string& defaultVal
) const {
	if (!m_parentCfg) return defaultVal;
	return m_parentCfg->getString(m_rootPath + "." + path, defaultVal);
}

std::vector<std::string> SubGameConfig::getStringArray(
	const std::string& path,
	const std::vector<std::string>& defaultVal
) const {
	if (!m_parentCfg) return defaultVal;
	return m_parentCfg->getStringArray(m_rootPath + "." + path, defaultVal);
}

Vector3 SubGameConfig::getVector3(const std::string& path, Vector3 defaultVal) const {
	if (!m_parentCfg) return defaultVal;
	return m_parentCfg->getVector3(m_rootPath + "." + path, defaultVal);
}
