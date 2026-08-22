#include "SoundManager.hpp"
#include "GameConfig.hpp"
#include <algorithm>
#include <iostream>

SoundManager::SoundManager() = default;

SoundManager::~SoundManager() {
	shutdown();
}

void SoundManager::init(const GameConfig& config) {
	if (!IsAudioDeviceReady()) {
		InitAudioDevice();
	}
	m_masterVolume = config.audio.masterVolume;
	SetMasterVolume(m_masterVolume);
	loadGlassSounds("assets/sounds");
	m_bgmStream = LoadMusicStream(config.audio.bgmPath.c_str());
	m_bgmLoaded = (m_bgmStream.frameCount > 0);
	m_initialized = true;
}

void SoundManager::shutdown() {
	if (!m_initialized) return;

	stopMusic();
	if (m_bgmLoaded) {
		UnloadMusicStream(m_bgmStream);
		m_bgmLoaded = false;
	}

	for (auto& [id, sound] : m_sounds) {
		UnloadSound(sound);
	}
	m_sounds.clear();
	m_pathCache.clear();
	m_glassSounds.clear();

	if (IsAudioDeviceReady()) {
		CloseAudioDevice();
	}
	m_initialized = false;
}

sound::Id SoundManager::loadSound(const std::string& path) {
	const auto it = m_pathCache.find(path);
	if (it != m_pathCache.end()) {
		return it->second;
	}

	if (!FileExists(path.c_str())) {
		return sound::NONE;
	}

	const Sound snd = LoadSound(path.c_str());
	if (snd.frameCount == 0) {
		return sound::NONE;
	}

	const sound::Id id = m_nextSoundId++;
	m_sounds[id] = snd;
	m_pathCache[path] = id;
	return id;
}

void SoundManager::loadGlassSounds(const std::string& dir) {
	m_glassSounds.clear();
	for (int i = 1; i <= 24; ++i) {
		char filename[64];
		std::snprintf(filename, sizeof(filename), "/glass_clink_%02d.wav", i);
		const std::string path = dir + filename;
		const sound::Id id = loadSound(path);
		if (id != sound::NONE) {
			m_glassSounds.push_back(id);
		}
	}
}

sound::Id SoundManager::getRandomGlassSound() const {
	if (m_glassSounds.empty()) {
		return sound::NONE;
	}
	const int index = GetRandomValue(0, static_cast<int>(m_glassSounds.size()) - 1);
	return m_glassSounds[index];
}

void SoundManager::unloadSound(sound::Id id) {
	const auto it = m_sounds.find(id);
	if (it == m_sounds.end()) return;

	UnloadSound(it->second);
	m_sounds.erase(it);
}

void SoundManager::queueSound(sound::Id id, Vector3 position, float volume) {
	if (!m_enabled || id == sound::NONE) return;
	m_pendingRequests.push_back({id, position, volume});
}

void SoundManager::playImmediate(sound::Id id, float volume) {
	if (!m_enabled || id == sound::NONE) return;
	const auto it = m_sounds.find(id);
	if (it == m_sounds.end()) return;

	SetSoundVolume(it->second, std::clamp(volume, 0.0f, 1.0f));
	PlaySound(it->second);
}

void SoundManager::update([[maybe_unused]] const Camera3D& camera) {
	if (!m_enabled) {
		m_pendingRequests.clear();
		return;
	}

	for (const auto& req : m_pendingRequests) {
		const auto it = m_sounds.find(req.id);
		if (it == m_sounds.end()) {
			continue;
		}

		float volume = req.volume;
		if (Vector3Length(req.position) > 0.001f) {
			const float distance = Vector3Distance(camera.position, req.position);
			const float attenuation = 1.0f / (1.0f + 0.002f * distance);
			volume *= attenuation;
		}

		SetSoundVolume(it->second, std::clamp(volume, 0.0f, 1.0f));
		PlaySound(it->second);
	}
	m_pendingRequests.clear();
}

void SoundManager::playMusic() {
	if (m_bgmLoaded && !IsMusicStreamPlaying(m_bgmStream)) {
		PlayMusicStream(m_bgmStream);
	}
}

void SoundManager::updateMusic() {
	if (m_bgmLoaded && IsMusicStreamPlaying(m_bgmStream)) {
		UpdateMusicStream(m_bgmStream);
	}
}

void SoundManager::stopMusic() {
	if (m_bgmLoaded) {
		StopMusicStream(m_bgmStream);
	}
}

void SoundManager::setMasterVolume(float volume) {
	m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
	SetMasterVolume(m_masterVolume);
}

float SoundManager::getMasterVolume() const {
	return m_masterVolume;
}

void SoundManager::setEnabled(bool val) {
	m_enabled = val;
	if (!m_enabled) {
		stopMusic();
	}
}

bool SoundManager::isEnabled() const {
	return m_enabled;
}
