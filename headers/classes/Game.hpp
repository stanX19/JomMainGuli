#pragma once

#include "game_context.hpp"
#include "Renderer.hpp"
#include "systems.hpp"

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

	systems::PlayerMoveControl m_playerMoveControl;
	systems::EntityMovement m_entityMovement;
	systems::DetectEntityCollision m_detectEntityCollision;
	systems::CameraFollowPlayer m_cameraFollowPlayer;
	systems::SpawnTrailParticles m_spawnTrailParticles;
	systems::EntityTransformation m_entityTransformation;
	systems::EntityLifetime m_entityLifetime;

	void inputControls([[maybe_unused]] float dt, EngineState &nextState);
};
