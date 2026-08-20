#include "Renderer.hpp"
#include "constants.hpp"
#include "rlgl.h"
#include <algorithm>
#include <iostream>

using namespace component;

Renderer::Renderer(Camera3D &cam, GameContext &context)
	: m_camera(cam), m_context(context)
{
	loadDefaultShader();
	loadShaderWithFallback();
	loadGlassShader();
	setupShaderUniforms();
	m_glassSphereModelId = m_context.modelManager.createSphere(24, 24, 1.0f);
}

Renderer::~Renderer()
{
	if (m_lightedShader.id != 0)
	{
		UnloadShader(m_lightedShader);
		m_lightedShader = {0, nullptr};
	}
	if (m_glassShader.id != 0)
	{
		UnloadShader(m_glassShader);
		m_glassShader = {0, nullptr};
	}
	if (m_defaultShader.id != 0)
	{
		UnloadShader(m_defaultShader);
		m_defaultShader = {0, nullptr};
	}
}

void Renderer::loadDefaultShader()
{
	m_defaultShader = LoadShader(nullptr, nullptr);
}

void Renderer::loadShaderWithFallback()
{
	m_lightedShader = LoadShader("shaders/sunlight.vs", "shaders/sunlight.fs");

	if (m_lightedShader.id == 0)
	{
		TraceLog(LOG_WARNING, "Custom shader failed to load. Using default shader.");
		UnloadShader(m_lightedShader);
		m_lightedShader = LoadShader(nullptr, nullptr);
	}
}

void Renderer::loadGlassShader()
{
	m_glassShader = LoadShader("shaders/glass.vs", "shaders/glass.fs");

	if (m_glassShader.id == 0)
	{
		TraceLog(LOG_WARNING, "Glass shader failed to load. Using default shader.");
		UnloadShader(m_glassShader);
		m_glassShader = LoadShader(nullptr, nullptr);
	}
}

void Renderer::setupShaderUniforms()
{
	m_lightPosLoc = GetShaderLocation(m_lightedShader, "lightPosition");
	m_lightColorLoc = GetShaderLocation(m_lightedShader, "lightColor");

	Vector3 lightPos = { 1000.0f, 1000.0f, 1000.0f };
	SetShaderValue(m_lightedShader, m_lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);

	Vector3 lightColor = { 1.0f, 1.0f, 1.0f };
	SetShaderValue(m_lightedShader, m_lightColorLoc, &lightColor, SHADER_UNIFORM_VEC3);

	m_cameraPosLoc = GetShaderLocation(m_glassShader, "cameraPosition");
	SetShaderValue(m_glassShader, GetShaderLocation(m_glassShader, "lightPosition"), &lightPos, SHADER_UNIFORM_VEC3);
	SetShaderValue(m_glassShader, GetShaderLocation(m_glassShader, "lightColor"), &lightColor, SHADER_UNIFORM_VEC3);
}

void Renderer::updateFrustum()
{
	const Matrix viewMat = rlGetMatrixModelview();
	const Matrix projMat = rlGetMatrixProjection();
	const Matrix viewProjMat = MatrixMultiply(viewMat, projMat);
	m_currentFrustum = Frustum::fromViewProjection(viewProjMat);
}

bool Renderer::isEntityVisible(const Position &pos, const RenderBody &body) const
{
	const float baseRadius = m_context.modelManager.getModelRadius(body.modelID);
	const float maxScale = std::max({body.scale.x, body.scale.y, body.scale.z, 0.01f});
	const float translationLen = Vector3Length(body.translation);
	const float effectiveRadius = baseRadius * maxScale + translationLen;
	return m_currentFrustum.isSphereInside(pos.value, effectiveRadius);
}

void Renderer::render(float dt)
{
	m_currentDt = dt;
	ClearBackground(BLACK);

	if (m_glassShader.id != 0 && m_cameraPosLoc != -1)
	{
		SetShaderValue(m_glassShader, m_cameraPosLoc, &m_camera.position, SHADER_UNIFORM_VEC3);
	}

	BeginMode3D(m_camera);
	updateFrustum();

	drawMap();
	drawEntities();
	drawTrails();

	EndMode3D();
}

void Renderer::drawMap()
{
	const auto mapModelId = m_context.map.getModelId();
	if (!mapModelId.has_value() || !m_context.modelManager.isValid(*mapModelId))
		return;

	Model &model = m_context.modelManager.getModel(*mapModelId);
	for (int i = 0; i < model.materialCount; i++) {
		model.materials[i].shader = (m_lightedShader.id != 0) ? m_lightedShader : m_defaultShader;
	}
	DrawModel(model, Vector3{0.0f, 0.0f, 0.0f}, 1.0f, WHITE);

	const float holeRadius = m_context.map.getHoleRadius();
	for (const auto &hole : m_context.map.getHoleCords()) {
		const Vector3 holeCenter = m_context.map.gridToWorld(hole);
		DrawCircle3D(holeCenter + Vector3{0.0f, 0.1f, 0.0f}, holeRadius, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, SKYBLUE);
		DrawCircle3D(holeCenter + Vector3{0.0f, 0.1f, 0.0f}, holeRadius + 0.1, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, SKYBLUE);
		DrawCircle3D(holeCenter + Vector3{0.0f, 0.1f, 0.0f}, holeRadius + 0.2, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, SKYBLUE);
		DrawCircle3D(holeCenter + Vector3{0.0f, 50.0f, 0.0f}, holeRadius, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, SKYBLUE);
		DrawCircle3D(holeCenter + Vector3{0.0f, 100.0f, 0.0f}, holeRadius, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, SKYBLUE);
		DrawCircle3D(holeCenter + Vector3{0.0f, 150.3f, 0.0f}, holeRadius, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, SKYBLUE);
	}
}

void Renderer::drawTrails()
{
	auto trailView = m_context.registry.view<Position, PrevPosition, Trail>();
	for (auto entity : trailView)
	{
		const Position &p = trailView.get<Position>(entity);
		const PrevPosition &pp = trailView.get<PrevPosition>(entity);
		const Trail &t = trailView.get<Trail>(entity);
		drawTrailBetween(p.value, pp.value, t.rad, t.color);
	}
}

void Renderer::drawTrailBetween(const Vector3 &head, const Vector3 &tail, float rad, Color color) const
{
	const Vector3 dir = head - tail;
	const float len = Vector3Length(dir);
	if (len < constants::epsilon)
		return;

	DrawCylinderEx(tail, head, rad, rad * 0.5f, 8, color);
}

void Renderer::drawEntityModel(const Position &pos, const RenderBody &body, const Quaternion &entityRot)
{
	if (!m_context.modelManager.isValid(body.modelID))
		return;

	Model &model = m_context.modelManager.getModel(body.modelID);

	const Quaternion finalRot = QuaternionNormalize(QuaternionMultiply(entityRot, body.rotation));
	Vector3 axis;
	float angle;
	QuaternionToAxisAngle(finalRot, &axis, &angle);

	const Vector3 position = pos.value + Vector3RotateByQuaternion(body.translation, finalRot);
	DrawModelEx(model, position, axis, angle * RAD2DEG, body.scale, body.color);
}

void Renderer::drawGlassShell(const Position &pos, const RenderBody &body, const Quaternion &entityRot)
{
	if (!m_context.modelManager.isValid(m_glassSphereModelId))
		return;

	Model &glassModel = m_context.modelManager.getModel(m_glassSphereModelId);
	const Shader activeShader = (m_glassShader.id != 0) ? m_glassShader : m_defaultShader;
	for (int i = 0; i < glassModel.materialCount; i++) {
		glassModel.materials[i].shader = activeShader;
	}

	constexpr Color CLASSICAL_GLASS_TINT = {215, 245, 240, 110};

	const Quaternion finalRot = QuaternionNormalize(QuaternionMultiply(entityRot, body.rotation));
	Vector3 axis;
	float angle;
	QuaternionToAxisAngle(finalRot, &axis, &angle);

	const Vector3 position = pos.value + Vector3RotateByQuaternion(body.translation, finalRot);
	DrawModelEx(glassModel, position, axis, angle * RAD2DEG, body.scale, CLASSICAL_GLASS_TINT);
}

void Renderer::drawEntities()
{
	auto view = m_context.registry.view<Position, RenderBody>();

	// Pass 1: Draw inner / opaque entity models with standard lighting
	for (auto entity : view)
	{
		const Position &pos = view.get<Position>(entity);
		const RenderBody &body = view.get<RenderBody>(entity);
		if (!isEntityVisible(pos, body))
			continue;

		if (m_context.modelManager.isValid(body.modelID))
		{
			Model &model = m_context.modelManager.getModel(body.modelID);
			const Shader activeShader = (m_lightedShader.id != 0) ? m_lightedShader : m_defaultShader;
			for (int i = 0; i < model.materialCount; i++) {
				model.materials[i].shader = activeShader;
			}
		}
		const auto *rot = m_context.registry.try_get<Rotation>(entity);
		const Quaternion entityRot = rot ? rot->value : QuaternionIdentity();
		drawEntityModel(pos, body, entityRot);
	}

	// Pass 2: Draw outer glass shells on top for glass-encased entities
	auto glassView = m_context.registry.view<Position, RenderBody, component::tags::IsCoveredByGlass>();
	for (auto entity : glassView)
	{
		const Position &pos = glassView.get<Position>(entity);
		const RenderBody &body = glassView.get<RenderBody>(entity);
		if (!isEntityVisible(pos, body))
			continue;

		const auto *rot = m_context.registry.try_get<Rotation>(entity);
		const Quaternion entityRot = rot ? rot->value : QuaternionIdentity();
		drawGlassShell(pos, body, entityRot);
	}
}
