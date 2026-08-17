#pragma once

#include "includes.hpp"
#include "op_overloads.hpp"

#include <algorithm>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using ModelId = std::size_t;

class GameConfig;

class ModelManager {
public:
  ModelManager();
  ~ModelManager();

  ModelId loadModel(const std::string &filePath);
  ModelId loadModel(const std::string &filePath, float scale);
  ModelId loadModel(const std::string &filePath, const Vector3 &scale);
  ModelId loadModel(const std::string &filePath, const Vector3 &scale, const Vector3 &rotation, const Vector3 &displacement);
  ModelId loadModel(const std::string &filePath, const Matrix &transform);
  ModelId loadModel(const GameConfig &config, const std::string &configPath);
  ModelId loadModel(Mesh mesh);

  ModelId createCube(float width = 2.0f, float height = 2.0f, float length = 2.0f);
  ModelId createSphere(int rings = 16, int slices = 16, float radius = 1.0f);
  ModelId createCylinder(int slices = 16, float radius = 1.0f, float height = 2.0f);
  ModelId createPlane(float width = 2.0f, float length = 2.0f, int resX = 4, int resZ = 4);

  Model &getModel(ModelId id);
  const Model &getModel(ModelId id) const;
  std::optional<std::string> getModelPath(ModelId id) const;

  void unloadAll();
  float getModelRadius(ModelId id) const;

  bool isValid(ModelId id) const;

private:
  std::vector<Model> m_models;
  std::vector<float> m_modelRadii;
  std::vector<std::optional<std::string>> m_modelPaths;
  std::map<std::pair<std::string, Matrix>, ModelId> m_loadedFromFile;
  std::map<std::string, ModelId> m_proceduralCache;

  float computeModelBoundingRadius(const Model &model) const;

  template <typename... Args>
  std::string generateCacheKey(const std::string &keyBase, Args &&...args) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << keyBase;
    ((ss << "_" << args), ...);
    return ss.str();
  }

  template <typename Func, typename... Args>
  ModelId createAndAddModel(const std::string &keyBase, Func modelGenerator, Args &&...args) {
    const std::string key = generateCacheKey(keyBase, args...);

    const auto it = m_proceduralCache.find(key);
    if (it != m_proceduralCache.end()) {
      return it->second;
    }

    Model model = modelGenerator();
    const ModelId id = m_models.size();
    m_models.push_back(model);
    m_modelRadii.push_back(computeModelBoundingRadius(model));
    m_modelPaths.emplace_back(std::nullopt);
    m_proceduralCache[key] = id;
    return id;
  }
};
