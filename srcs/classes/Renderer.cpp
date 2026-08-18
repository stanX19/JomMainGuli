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
	setupShaderUniforms();
}

Renderer::~Renderer()
{
	if (m_lightedShader.id != 0)
	{
		UnloadShader(m_lightedShader);
		m_lightedShader = {0, nullptr};
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

void Renderer::setupShaderUniforms()
{
	m_lightPosLoc = GetShaderLocation(m_lightedShader, "lightPosition");
	m_lightColorLoc = GetShaderLocation(m_lightedShader, "lightColor");

	Vector3 lightPos = { 1000.0f, 1000.0f, 1000.0f };
	SetShaderValue(m_lightedShader, m_lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);

	Vector3 lightColor = { 1.0f, 1.0f, 1.0f };
	SetShaderValue(m_lightedShader, m_lightColorLoc, &lightColor, SHADER_UNIFORM_VEC3);
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

void Renderer::drawEntityModel(const Position &pos, const RenderBody &body)
{
	if (!m_context.modelManager.isValid(body.modelID))
		return;

	Model &model = m_context.modelManager.getModel(body.modelID);

	Vector3 axis;
	float angle;
	QuaternionToAxisAngle(body.rotation, &axis, &angle);

	const Vector3 position = pos.value + Vector3RotateByQuaternion(body.translation, body.rotation);
	DrawModelEx(model, position, axis, angle * RAD2DEG, body.scale, body.color);
}

void Renderer::drawEntities()
{
	auto view = m_context.registry.view<Position, RenderBody>();

	for (auto entity : view)
	{
		const Position &pos = view.get<Position>(entity);
		const RenderBody &body = view.get<RenderBody>(entity);
		if (!isEntityVisible(pos, body))
			continue;

		if (m_context.modelManager.isValid(body.modelID))
		{
			Model &model = m_context.modelManager.getModel(body.modelID);
			for (int i = 0; i < model.materialCount; i++) {
				model.materials[i].shader = (m_lightedShader.id != 0) ? m_lightedShader : m_defaultShader;
			}
		}
		drawEntityModel(pos, body);
	}
}
