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

	m_context.map = map::MapLoader::load(m_context.config, "assets/maps/level2.map");
	map::MapMeshGenerator(m_context.map).generateAndAssignModel(m_context.modelManager);
	m_context.map.spawnAll(m_context);

	const auto &initEntities = m_context.map.getInitialEntities();
	const auto guliCount = std::count_if(initEntities.begin(), initEntities.end(), [](const auto &e) {
		return e.type == map::EntityType::Guli;
	});
	const int totalGuli = (guliCount > 0) ? static_cast<int>(guliCount) : 5;
	m_context.state = GameState(totalGuli);
}

EngineState Game::run() {
	m_context.soundManager.playMusic();
	EngineState nextState = EngineState::GAME;

	while (!WindowShouldClose() && nextState == EngineState::GAME) {
		const float dt = GetFrameTime();
		m_context.soundManager.updateMusic();

		m_gameStateSystem.update(m_context, dt);
		m_playerMoveControl.update(m_context, dt);
		m_playerMagicCast.update(m_context, dt);
		m_particleAttraction.update(m_context, dt);
		m_detectEntityCollision.update(m_context, dt);
		m_context.dispatcher.update();
		m_entityWorldCollision.update(m_context, dt); // right before movement update
		m_entityMovement.update(m_context, dt);
		
		m_guliInHoleSystem.update(m_context, dt);
		m_context.soundManager.update(m_context.mainCamera);

		m_cameraFollowPlayer.update(m_context, dt);
		m_guliView.update(m_context, dt);
		m_spawnTrailParticles.update(m_context, dt);
		m_entityTransformation.update(m_context, dt);
		m_entityLifetime.update(m_context, dt);

		BeginDrawing();
		m_renderer.render(dt);
		m_hud.render(m_context, dt);
		EndDrawing();

		inputControls(dt, nextState);
	}

	m_context.soundManager.stopMusic();
	return nextState;
}

void Game::inputControls([[maybe_unused]] float dt, EngineState &nextState) {
	if (IsKeyPressed(KEY_H)) {
		m_context.state.showHelpOverlay = !m_context.state.showHelpOverlay;
		return;
	}

	if (IsKeyPressed(KEY_ESCAPE)) {
		if (m_context.state.showHelpOverlay) {
			m_context.state.showHelpOverlay = false;
			return;
		}

		if (m_context.registry.valid(m_context.currentPlayer) && m_context.registry.all_of<component::GuliViewState>(m_context.currentPlayer)) {
			m_context.registry.remove<component::GuliViewState>(m_context.currentPlayer);
			return;
		}
		nextState = EngineState::MENU;
		return;
	}

	if (IsKeyPressed(KEY_R)) {
		reset();
	}
}

