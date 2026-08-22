#pragma once

#include "includes.hpp"
#include <map>
#include <string>
#include <vector>

namespace sound {
	using Id = int;
	constexpr Id NONE = 0;

	struct PlaySoundRequest {
		Id id = NONE;
		Vector3 position{0.0f, 0.0f, 0.0f};
		float volume = 1.0f;
	};
} // namespace sound

class GameConfig;

class SoundManager {
public:
	SoundManager();
	~SoundManager();

	void init(const GameConfig& config);
	void shutdown();

	// Process queued sounds
	void update(const Camera3D& camera);

	void queueSound(sound::Id id, Vector3 position = {0.0f, 0.0f, 0.0f}, float volume = 1.0f);
	void playImmediate(sound::Id id, float volume = 1.0f);

	sound::Id loadSound(const std::string& path);
	void loadGlassSounds(const std::string& dir = "assets/sounds/glass_clink");
	sound::Id getRandomGlassSound() const;

	sound::Id getShootSound() const;
	sound::Id getGuliMergeSound() const;
	sound::Id getCollectGuliSound() const;
	sound::Id getVictorySound() const;
	sound::Id getWinSound() const;

	// Background music
	void playMusic();
	void updateMusic();
	void stopMusic();

	void setMasterVolume(float volume);
	float getMasterVolume() const;

	void setEnabled(bool enabled);
	bool isEnabled() const;

private:
	std::map<sound::Id, Sound> m_sounds;
	std::map<std::string, sound::Id> m_pathCache;
	std::vector<sound::Id> m_glassSounds;
	std::vector<sound::PlaySoundRequest> m_pendingRequests;

	sound::Id m_shootSoundId = sound::NONE;
	sound::Id m_guliMergeSoundId = sound::NONE;
	sound::Id m_collectGuliSoundId = sound::NONE;
	sound::Id m_victorySoundId = sound::NONE;

	Music m_bgmStream{};
	bool m_bgmLoaded = false;

	float m_masterVolume = 0.5f;
	bool m_enabled = true;
	bool m_initialized = false;
	int m_nextSoundId = 1;

	void unloadSound(sound::Id id);
};
