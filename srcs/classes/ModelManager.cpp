#include "ModelManager.hpp"
#include "GameConfig.hpp"
#include "utils.hpp"
#include "rlgl.h"
#include <filesystem>
#include <iostream>
#include <stdexcept>

ModelManager::ModelManager() = default;

ModelManager::~ModelManager() { unloadAll(); }

namespace
{
	void prepareTexture(Texture2D &texture)
	{
		if (texture.id == 0 || texture.width <= 1 || texture.height <= 1)
			return;
		if (texture.mipmaps <= 1)
			GenTextureMipmaps(&texture);
		if (texture.mipmaps > 1)
			SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
	}

	void prepareModelTextures(Model &model)
	{
		for (int i = 0; i < model.materialCount; ++i)
		{
			Material &material = model.materials[i];
			if (material.maps == nullptr)
				continue;
			prepareTexture(material.maps[MATERIAL_MAP_DIFFUSE].texture);
			prepareTexture(material.maps[MATERIAL_MAP_NORMAL].texture);
		}
	}
} // namespace

float ModelManager::computeModelBoundingRadius(const Model &model) const
{
	const BoundingBox box = GetModelBoundingBox(model);
	float maxDistSq = 0.0f;
	for (const int x : {0, 1})
	{
		for (const int y : {0, 1})
		{
			for (const int z : {0, 1})
			{
				const Vector3 pt = {x ? box.max.x : box.min.x,
									y ? box.max.y : box.min.y,
									z ? box.max.z : box.min.z};
				const float distSq = pt.x * pt.x + pt.y * pt.y + pt.z * pt.z;
				if (distSq > maxDistSq)
					maxDistSq = distSq;
			}
		}
	}
	return std::max(0.01f, std::sqrt(maxDistSq));
}

ModelId ModelManager::loadModel(const std::string &filePath,
								const Matrix &transform)
{
	const auto key = std::make_pair(filePath, transform);
	const auto it = m_loadedFromFile.find(key);
	if (it != m_loadedFromFile.end())
	{
		return it->second;
	}

	const std::filesystem::path originalPath = std::filesystem::current_path();
	const std::filesystem::path modelPath = std::filesystem::absolute(filePath);
	const std::filesystem::path modelDir = modelPath.parent_path();
	const std::filesystem::path modelFile = modelPath.filename();

	if (!std::filesystem::exists(modelDir))
	{
		throw std::runtime_error("Model directory does not exist: " +
								 modelDir.string());
	}

	std::filesystem::current_path(modelDir);
	Model model = LoadModel(modelFile.string().c_str());
	std::filesystem::current_path(originalPath);
	prepareModelTextures(model);

	model.transform = transform;

	m_models.push_back(model);
	m_modelRadii.push_back(computeModelBoundingRadius(model));
	m_modelPaths.emplace_back(filePath);
	const ModelId id = m_models.size() - 1;
	m_loadedFromFile[key] = id;
	return id;
}

ModelId ModelManager::loadModel(const std::string &filePath,
								const Vector3 &scale, const Vector3 &rotation,
								const Vector3 &displacement)
{
	const Matrix transform =
		utils::math::getTransformMatrix(scale, rotation, displacement);
	return loadModel(filePath, transform);
}

ModelId ModelManager::loadModel(const std::string &filePath,
								const Vector3 &scale)
{
	const Vector3 rotation = {0.0f, 0.0f, 0.0f};
	const Vector3 displacement = {0.0f, 0.0f, 0.0f};
	return loadModel(filePath, scale, rotation, displacement);
}

ModelId ModelManager::loadModel(const std::string &filePath, float scale)
{
	const Vector3 scaleVec = {scale, scale, scale};
	return loadModel(filePath, scaleVec);
}

ModelId ModelManager::loadModel(const std::string &filePath)
{
	const Matrix identityMatrix = MatrixIdentity();
	return loadModel(filePath, identityMatrix);
}

ModelId ModelManager::loadModel(const GameConfig &config,
								const std::string &configPath)
{
	const std::string filePath = config.getString(configPath, "");
	return loadModel(filePath);
}

ModelId ModelManager::createCube(float width, float height, float length)
{
	return createAndAddModel(
		"box",
		[=]()
		{
			const Mesh mesh = GenMeshCube(width, height, length);
			return LoadModelFromMesh(mesh);
		},
		width, height, length);
}

ModelId ModelManager::createSphere(int rings, int slices, float radius)
{
	return createAndAddModel(
		"sphere",
		[=]()
		{
			const Mesh mesh = GenMeshSphere(radius, rings, slices);
			return LoadModelFromMesh(mesh);
		},
		radius, rings, slices);
}

ModelId ModelManager::createCylinder(int slices, float radius, float height)
{
	return createAndAddModel(
		"cylinder",
		[=]()
		{
			const Mesh mesh = GenMeshCylinder(radius, height, slices);
			Model model = LoadModelFromMesh(mesh);
			const Matrix transform = utils::math::getTransformMatrix(
				Vector3{1.0f, 1.0f, 1.0f}, Vector3{90.0f, 0.0f, 0.0f} * DEG2RAD,
				Vector3{0.0f, 0.0f, -height / 2.0f});
			model.transform = transform;
			return model;
		},
		radius, height, slices);
}

ModelId ModelManager::createPlane(float width, float length, int resX,
								  int resZ)
{
	return createAndAddModel(
		"plane",
		[=]()
		{
			const Mesh mesh = GenMeshPlane(width, length, resX, resZ);
			return LoadModelFromMesh(mesh);
		},
		width, length, resX, resZ);
}

ModelId ModelManager::loadModel(Mesh mesh)
{
	Model model = LoadModelFromMesh(mesh);
	const ModelId id = m_models.size();
	m_models.push_back(model);
	m_modelRadii.push_back(computeModelBoundingRadius(model));
	m_modelPaths.emplace_back(std::nullopt);
	return id;
}

Model &ModelManager::getModel(ModelId id)
{
	if (!isValid(id))
	{
		throw std::out_of_range("Invalid model ID");
	}
	return m_models[id];
}

const Model &ModelManager::getModel(ModelId id) const
{
	if (!isValid(id))
	{
		throw std::out_of_range("Invalid model ID");
	}
	return m_models[id];
}

std::optional<std::string> ModelManager::getModelPath(ModelId id) const
{
	if (!isValid(id))
	{
		throw std::out_of_range("Invalid model ID");
	}
	return m_modelPaths[id];
}

void ModelManager::unloadAll()
{
	for (auto &model : m_models)
	{
		for (int i = 0; i < model.materialCount; ++i)
		{
			if (model.materials[i].maps != nullptr)
			{
				Texture2D &diffuseTex = model.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture;
				if (diffuseTex.id > 1)
				{
					UnloadTexture(diffuseTex);
					diffuseTex = {0, 0, 0, 0, 0};
				}
			}
			model.materials[i].shader = {rlGetShaderIdDefault(), rlGetShaderLocsDefault()};
		}
		UnloadModel(model);
	}
	m_models.clear();
	m_modelRadii.clear();
	m_modelPaths.clear();
	m_proceduralCache.clear();
	m_loadedFromFile.clear();
}

bool ModelManager::isValid(ModelId id) const { return id < m_models.size(); }

float ModelManager::getModelRadius(ModelId id) const
{
	if (id < m_modelRadii.size())
	{
		return m_modelRadii[id];
	}
	return 1.0f;
}

Mesh ModelManager::generateMeshFromTriangles(const std::vector<Triangle> &triangles, bool hasTexCoords)
{
	Mesh mesh{};
	const int triangleCount = static_cast<int>(triangles.size());
	if (triangleCount == 0)
	{
		return mesh;
	}

	const int vertexCount = triangleCount * 3;
	mesh.vertexCount = vertexCount;
	mesh.triangleCount = triangleCount;

	mesh.vertices = static_cast<float *>(MemAlloc(vertexCount * 3 * sizeof(float)));
	mesh.normals = static_cast<float *>(MemAlloc(vertexCount * 3 * sizeof(float)));
	mesh.colors = static_cast<unsigned char *>(MemAlloc(vertexCount * 4 * sizeof(unsigned char)));
	if (hasTexCoords)
	{
		mesh.texcoords = static_cast<float *>(MemAlloc(vertexCount * 2 * sizeof(float)));
	}

	int vIdx = 0;
	for (const auto &tri : triangles)
	{
		for (int i = 0; i < 3; ++i)
		{
			const Vertex &vd = tri.v[i];
			mesh.vertices[vIdx * 3 + 0] = vd.pos.x;
			mesh.vertices[vIdx * 3 + 1] = vd.pos.y;
			mesh.vertices[vIdx * 3 + 2] = vd.pos.z;

			mesh.normals[vIdx * 3 + 0] = vd.normal.x;
			mesh.normals[vIdx * 3 + 1] = vd.normal.y;
			mesh.normals[vIdx * 3 + 2] = vd.normal.z;

			mesh.colors[vIdx * 4 + 0] = vd.color.r;
			mesh.colors[vIdx * 4 + 1] = vd.color.g;
			mesh.colors[vIdx * 4 + 2] = vd.color.b;
			mesh.colors[vIdx * 4 + 3] = vd.color.a;

			if (hasTexCoords)
			{
				mesh.texcoords[vIdx * 2 + 0] = vd.uv.x;
				mesh.texcoords[vIdx * 2 + 1] = vd.uv.y;
			}
			++vIdx;
		}
	}

	UploadMesh(&mesh, false);
	return mesh;
}

Mesh ModelManager::generateMeshFromVerts(const std::vector<Vertex> &verts, bool hasTexCoords)
{
	Mesh mesh{};
	const int count = static_cast<int>(verts.size());
	if (count == 0)
	{
		return mesh;
	}

	mesh.vertexCount = count;
	mesh.triangleCount = count / 3;

	mesh.vertices = static_cast<float *>(MemAlloc(count * 3 * sizeof(float)));
	mesh.normals = static_cast<float *>(MemAlloc(count * 3 * sizeof(float)));
	mesh.colors = static_cast<unsigned char *>(MemAlloc(count * 4 * sizeof(unsigned char)));
	if (hasTexCoords)
	{
		mesh.texcoords = static_cast<float *>(MemAlloc(count * 2 * sizeof(float)));
	}

	for (int i = 0; i < count; ++i)
	{
		mesh.vertices[i * 3 + 0] = verts[i].pos.x;
		mesh.vertices[i * 3 + 1] = verts[i].pos.y;
		mesh.vertices[i * 3 + 2] = verts[i].pos.z;

		mesh.normals[i * 3 + 0] = verts[i].normal.x;
		mesh.normals[i * 3 + 1] = verts[i].normal.y;
		mesh.normals[i * 3 + 2] = verts[i].normal.z;

		mesh.colors[i * 4 + 0] = verts[i].color.r;
		mesh.colors[i * 4 + 1] = verts[i].color.g;
		mesh.colors[i * 4 + 2] = verts[i].color.b;
		mesh.colors[i * 4 + 3] = verts[i].color.a;

		if (hasTexCoords)
		{
			mesh.texcoords[i * 2 + 0] = verts[i].uv.x;
			mesh.texcoords[i * 2 + 1] = verts[i].uv.y;
		}
	}

	UploadMesh(&mesh, false);
	return mesh;
}
