#pragma once
#include <string>

struct AudioClip;

class AudioManager {
public:
    static AudioManager& Get();

    bool Init();
    void Shutdown();

    void Play(AudioClip* clip, float volume = 1.0f, bool loop = false);
    void Stop();
    void PlayOneShot(AudioClip* clip, float volume = 1.0f);
    void SetMasterVolume(float volume);

private:
    AudioManager() = default;
    ~AudioManager();

    struct Impl;
    Impl* m_impl = nullptr;
};
