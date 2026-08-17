#pragma once

#include "includes.hpp"
#include <map>
#include <string>
#include <vector>

namespace sound {
	using Id = int;
	constexpr Id NONE = 0;
} // namespace sound

struct PlaySoundRequest {
	sound::Id id = sound::NONE;
	Vector3 position{0.0f, 0.0f, 0.0f};
	float volume = 1.0f;
};

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
	std::vector<PlaySoundRequest> m_pendingRequests;

	Music m_bgmStream{};
	bool m_bgmLoaded = false;

	float m_masterVolume = 0.5f;
	bool m_enabled = true;
	bool m_initialized = false;
	int m_nextSoundId = 1;

	void unloadSound(sound::Id id);
};
