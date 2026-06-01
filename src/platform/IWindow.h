#pragma once
#include "renderer/Framebuffer.h"

class IWindow {
public:
    virtual ~IWindow() = default;

    virtual bool IsOpen() const = 0;
    virtual void PollEvents() = 0;
    virtual void Present(const Framebuffer& fb) = 0;
    virtual float DeltaTime() const = 0;
    virtual int Width() const = 0;
    virtual int Height() const = 0;
};
