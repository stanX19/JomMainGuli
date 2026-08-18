#include "Game.hpp"
#include "Engine.hpp"
#include "entities.hpp"
#include "events.hpp"
#include "systems.hpp"
#include "map/MapLoader.hpp"
#include "map/MapMeshGenerator.hpp"

Game::Game(GameContext &context) 
	: m_context(context), 
	  m_renderer(context.mainCamera, context)
{}

Game::~Game() = default;

void Game::reset() {
	m_context.registry.clear();
	event::utils::hookAllListeners(m_context);

	m_context.map = map::MapLoader::load(m_context.config, "assets/maps/level1.map");
	map::MapMeshGenerator(m_context.map).generateAndAssignModel(m_context.modelManager);
	m_context.map.spawnAll(m_context);
}

EngineState Game::run() {
	m_context.soundManager.playMusic();
	EngineState nextState = EngineState::GAME;

	while (!WindowShouldClose() && nextState == EngineState::GAME) {
		const float dt = GetFrameTime();
		m_context.soundManager.updateMusic();

		m_playerMoveControl.update(m_context, dt);
		m_playerMagicCast.update(m_context, dt);
		m_entityMovement.update(m_context, dt);
		m_entityWorldCollision.update(m_context, dt);
		m_detectEntityCollision.update(m_context, dt);
		m_context.dispatcher.update();
		m_context.soundManager.update(m_context.mainCamera);

		m_cameraFollowPlayer.update(m_context, dt);
		m_spawnTrailParticles.update(m_context, dt);
		m_entityTransformation.update(m_context, dt);
		m_entityLifetime.update(m_context, dt);

		BeginDrawing();
		m_renderer.render(dt);
		EndDrawing();

		inputControls(dt, nextState);
	}

	m_context.soundManager.stopMusic();
	return nextState;
}

void Game::inputControls([[maybe_unused]] float dt, EngineState &nextState) {
	if (IsKeyPressed(KEY_ESCAPE)) {
		nextState = EngineState::MENU;
		return;
	}
	if (IsKeyPressed(KEY_R)) {
		reset();
	}
}
