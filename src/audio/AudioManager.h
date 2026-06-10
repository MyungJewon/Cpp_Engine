#pragma once
#include <mutex>
#include <string>
#include <vector>

struct AudioClip;
typedef struct ma_sound ma_sound;

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
    std::vector<ma_sound*> m_oneShots;
    std::mutex m_oneShotMutex;
};
