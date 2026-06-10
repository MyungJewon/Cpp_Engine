#pragma once

struct AudioClip;

struct AudioSource {
    AudioClip* clip        = nullptr;
    float      volume      = 1.0f;
    float      pitch       = 1.0f;
    bool       loop        = false;
    bool       playOnAwake = false;
};
