#include "systems/AudioSystem.h"

#include "audio/AudioManager.h"
#include "audio/AudioSource.h"
#include "ecs/Registry.hpp"

void AudioSystem::update(Registry& reg, float) {
    if (m_initialized) {
        return;
    }

    for (auto& audioSource : reg.pool<AudioSource>()) {
        if (audioSource.playOnAwake) {
            AudioManager::Get().Play(audioSource.clip, audioSource.volume, audioSource.loop);
        }
    }

    m_initialized = true;
}
