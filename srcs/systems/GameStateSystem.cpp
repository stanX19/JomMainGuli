#include "systems/GameStateSystem.hpp"
#include "GameContext.hpp"
#include "components.hpp"
#include "events.hpp"
#include <algorithm>
#include <string>

void systems::GameStateSystem::update(GameContext &context, float dt) {
	if (!context.state.isVictory && context.state.gameTimer > 0.0f) {
		context.state.gameTimer = std::max(0.0f, context.state.gameTimer - dt);
	}

	if (context.state.isVictory)
		return;

	if (!context.registry.valid(context.currentPlayer))
		return;

	const auto *inv = context.registry.try_get<component::GuliInventory>(context.currentPlayer);
	if (!inv)
		return;

	const int winTarget = std::min(5, std::max(1, context.state.totalGuliInMap));
	const int collected = static_cast<int>(inv->guliCollection.size());

	if (collected < winTarget)
		return;

	context.state.isVictory = true;

	const sound::Id victorySound = context.soundManager.getVictorySound();
	if (victorySound != sound::NONE) {
		context.dispatcher.trigger<event::SoundEvent>(event::SoundEvent{
			&context,
			victorySound,
			context.mainCamera.position,
			1.0f
		});
	}

	const std::string toastText = "Victory!\n" + std::to_string(collected) + "/" + std::to_string(winTarget) + " Guli Collected";
	context.state.addToast(toastText, Color{52, 211, 153, 255}, 5.0f, 44, ToastPriority::HIGH);
}
