// 플랫폼 창 구현이 제공해야 하는 공통 인터페이스를 선언합니다.
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

    virtual int PixelWidth()  const { return Width(); }
    virtual int PixelHeight() const { return Height(); }
};
