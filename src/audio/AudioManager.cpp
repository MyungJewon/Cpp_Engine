#define MINIAUDIO_IMPLEMENTATION
#include "vendor/miniaudio.h"

#include "audio/AudioClip.h"
#include "audio/AudioManager.h"

struct AudioManager::Impl {
    ma_engine engine;
    ma_sound sound;
    bool soundInitialized = false;
};

AudioManager& AudioManager::Get() {
    static AudioManager instance;
    return instance;
}

AudioManager::~AudioManager() {
    Shutdown();
}

bool AudioManager::Init() {
    if (m_impl) {
        return true;
    }

    Impl* impl = new Impl();
    if (ma_engine_init(nullptr, &impl->engine) != MA_SUCCESS) {
        delete impl;
        return false;
    }

    m_impl = impl;
    return true;
}

void AudioManager::Shutdown() {
    if (!m_impl) {
        return;
    }

    Stop();
    {
        std::lock_guard<std::mutex> lock(m_oneShotMutex);
        for (ma_sound* s : m_oneShots) {
            ma_sound_stop(s);
            ma_sound_uninit(s);
            delete s;
        }
        m_oneShots.clear();
    }
    ma_engine_uninit(&m_impl->engine);
    delete m_impl;
    m_impl = nullptr;
}

void AudioManager::Play(AudioClip* clip, float volume, bool loop) {
    if (!clip || !clip->IsValid()) {
        return;
    }

    if (!m_impl && !Init()) {
        return;
    }

    Stop();

    const ma_uint32 flags = loop ? MA_SOUND_FLAG_STREAM : 0;
    if (ma_sound_init_from_file(&m_impl->engine, clip->path.c_str(), flags, nullptr, nullptr, &m_impl->sound) != MA_SUCCESS) {
        return;
    }

    m_impl->soundInitialized = true;
    ma_sound_set_volume(&m_impl->sound, volume);
    ma_sound_set_looping(&m_impl->sound, loop ? MA_TRUE : MA_FALSE);
    ma_sound_start(&m_impl->sound);
}

void AudioManager::Stop() {
    if (!m_impl || !m_impl->soundInitialized) {
        return;
    }

    ma_sound_stop(&m_impl->sound);
    ma_sound_uninit(&m_impl->sound);
    m_impl->soundInitialized = false;
}

void AudioManager::PlayOneShot(AudioClip* clip, float volume) {
    if (!clip || !clip->IsValid()) return;
    if (!m_impl && !Init()) return;
    ma_sound* s = new ma_sound();
    if (ma_sound_init_from_file(&m_impl->engine, clip->path.c_str(), 0, nullptr, nullptr, s) == MA_SUCCESS) {
        ma_sound_set_volume(s, volume);
        ma_sound_start(s);
        std::lock_guard<std::mutex> lock(m_oneShotMutex);
        m_oneShots.push_back(s);
    } else {
        delete s;
    }
}

void AudioManager::SetMasterVolume(float volume) {
    if (!m_impl && !Init()) {
        return;
    }

    ma_engine_set_volume(&m_impl->engine, volume);
}
