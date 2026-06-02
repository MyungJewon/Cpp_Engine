#pragma once

class IWindow {
public:
    virtual ~IWindow() = default;

    virtual bool IsOpen() const = 0;
    virtual void PollEvents() = 0;
    virtual void SwapBuffers() = 0;
    virtual float DeltaTime() const = 0;
    virtual int Width() const = 0;
    virtual int Height() const = 0;
    // Retina 등 고밀도 디스플레이의 실제 픽셀 크기 (논리 크기와 다를 수 있음)
    virtual int PixelWidth()  const { return Width(); }
    virtual int PixelHeight() const { return Height(); }
};
