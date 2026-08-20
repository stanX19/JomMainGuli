#pragma once

#include "GameContext.hpp"
#include "Renderer.hpp"
#include "systems.hpp"
#include "ui/GameHUD.hpp"

enum class EngineState;

class Game {
public:
	Game(GameContext &context);
	~Game();

	void reset();
	EngineState run();

private:
	GameContext &m_context;
	Renderer m_renderer;
	ui::GameHUD m_hud;
	int m_totalGuliInMap = 5;

	systems::PlayerMoveControl m_playerMoveControl;
	systems::PlayerMagicCast m_playerMagicCast;
	systems::ParticleAttraction m_particleAttraction;
	systems::EntityMovement m_entityMovement;
	systems::EntityWorldCollision m_entityWorldCollision;
	systems::DetectEntityCollision m_detectEntityCollision;
	systems::CameraFollowPlayer m_cameraFollowPlayer;
	systems::SpawnTrailParticles m_spawnTrailParticles;
	systems::EntityTransformation m_entityTransformation;
	systems::EntityLifetime m_entityLifetime;
	systems::GuliInHoleSystem m_guliInHoleSystem;
	systems::GuliView m_guliView;

	void inputControls([[maybe_unused]] float dt, EngineState &nextState);
};

