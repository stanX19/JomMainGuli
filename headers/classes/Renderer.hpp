#pragma once

#include "includes.hpp"
#include "components.hpp"
#include "utils.hpp"
#include "GameContext.hpp"
#include "Frustum.hpp"

class Renderer {
public:
	Renderer(Camera3D& camera, GameContext &context);
	~Renderer();

	void render(float dt);

private:
	Camera3D& m_camera;
	GameContext &m_context;
	float m_currentDt = 0.0f;
	Shader m_lightedShader;
	Shader m_glassShader;
	Shader m_defaultShader;

	int m_lightPosLoc = 0;
	int m_lightColorLoc = 0;
	int m_cameraPosLoc = 0;
	Frustum m_currentFrustum;

	void loadDefaultShader();
	void loadShaderWithFallback();
	void loadGlassShader();
	void setupShaderUniforms();
	void updateFrustum();
	bool isEntityVisible(const component::Position &pos, const component::RenderBody &body) const;
	void drawEntityModel(const component::Position &pos, const component::RenderBody &body);
	void drawTrails();
	void drawTrailBetween(const Vector3 &head, const Vector3 &tail, float rad, Color color) const;
	void drawMap();
	void drawEntities();
};
