#include "platform/MacWindow.h"
#include "input/InputCodes.h"
#include "input/InputManager.h"
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl3.h>

// ─── GLView ───────────────────────────────────────────────────────────────────
// OpenGL 4.1 Core Profile 컨텍스트를 소유하는 NSOpenGLView
@interface GLView : NSOpenGLView
+ (NSOpenGLPixelFormat*)defaultPixelFormat;
@end

@implementation GLView

// 더블 버퍼, 24비트 깊이 버퍼, MSAA 4x가 적용된 Core Profile 픽셀 포맷 생성
+ (NSOpenGLPixelFormat*)defaultPixelFormat {
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAMultisample,
        NSOpenGLPFASampleBuffers, 1,
        NSOpenGLPFASamples, 4,
        NSOpenGLPFAAccelerated,
        0
    };
    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if (format) return format;

    // 일부 실행 환경에서는 MSAA 픽셀 포맷 생성이 실패하므로 기본 더블 버퍼 포맷으로 폴백한다.
    NSOpenGLPixelFormatAttribute fallbackAttrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 24,
        0
    };
    format = [[NSOpenGLPixelFormat alloc] initWithAttributes:fallbackAttrs];
    if (format) return format;

    NSOpenGLPixelFormatAttribute minimalAttrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 24,
        0
    };
    return [[NSOpenGLPixelFormat alloc] initWithAttributes:minimalAttrs];
}

// 수직 동기화 활성화
- (void)prepareOpenGL {
    [super prepareOpenGL];
    GLint swapInterval = 1;
    [[self openGLContext] setValues:&swapInterval forParameter:NSOpenGLContextParameterSwapInterval];
}

@end

// ─── WindowDelegate ───────────────────────────────────────────────────────────
// 창 닫기 이벤트를 C++ 측 m_open 플래그에 전달하기 위한 델리게이트
@interface WindowDelegate : NSObject<NSWindowDelegate> {
    bool* _openPtr;
}
- (instancetype)initWithOpenPtr:(bool*)openPtr;
@end

@implementation WindowDelegate

- (instancetype)initWithOpenPtr:(bool*)openPtr {
    self = [super init];
    if (self) {
        _openPtr = openPtr;
    }
    return self;
}

// 닫기 버튼 클릭 시 메인 루프 종료 신호 전달
- (BOOL)windowShouldClose:(NSWindow*)sender {
    if (_openPtr) *_openPtr = false;
    return YES;
}

// 창이 실제로 닫힐 때 한 번 더 플래그 보장
- (void)windowWillClose:(NSNotification*)n {
    if (_openPtr) *_openPtr = false;
}

@end

// ─── MacWindow ────────────────────────────────────────────────────────────────
struct MacWindow::Impl {
    NSWindow*       window   = nil;
    GLView*         glView   = nil;
    NSOpenGLContext* context = nil;
    WindowDelegate* delegate = nil;
};

// NSApplication 초기화, NSWindow + GLView 생성, OpenGL 상태 초기화
MacWindow::MacWindow(int width, int height, const char* title)
    : m_impl(std::make_unique<Impl>())
    , m_width(width)
    , m_height(height)
{
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];

    NSRect frame = NSMakeRect(0, 0, width, height);
    NSWindowStyleMask style = NSWindowStyleMaskTitled
                            | NSWindowStyleMaskClosable
                            | NSWindowStyleMaskMiniaturizable;

    m_impl->window = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:style
        backing:NSBackingStoreBuffered
        defer:NO];

    NSOpenGLPixelFormat* pixelFormat = [GLView defaultPixelFormat];
    m_impl->glView = [[GLView alloc] initWithFrame:frame pixelFormat:pixelFormat];
    m_impl->context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    [m_impl->glView setOpenGLContext:m_impl->context];

    m_impl->delegate = [[WindowDelegate alloc] initWithOpenPtr:&m_open];

    [m_impl->window setContentView:m_impl->glView];
    [m_impl->context setView:m_impl->glView];
    [m_impl->window setDelegate:m_impl->delegate];
    [m_impl->window setTitle:@(title)];
    [m_impl->window setAcceptsMouseMovedEvents:YES];
    [m_impl->window setReleasedWhenClosed:NO];
    [m_impl->window center];
    [m_impl->window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];

    [m_impl->context makeCurrentContext];
    [m_impl->context update];
    [m_impl->glView prepareOpenGL];

    // Retina 디스플레이의 실제 픽셀 크기 계산 (논리 크기 × 배율)
    NSRect backingBounds = [m_impl->glView convertRectToBacking:
                            [m_impl->glView bounds]];
    m_pixelWidth  = (int)backingBounds.size.width;
    m_pixelHeight = (int)backingBounds.size.height;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glViewport(0, 0, m_pixelWidth, m_pixelHeight);

    m_lastTime = std::chrono::steady_clock::now();
}

// 창 리소스 해제
MacWindow::~MacWindow() {
    if (m_impl->window) [m_impl->window close];
}

// 블로킹 없이 이벤트 큐를 비우고 ESC 입력 처리, 델타타임 갱신
// 렌더 루프 최상단에서 매 프레임 호출
void MacWindow::PollEvents() {
    NSEvent* e;
    while ((e = [NSApp nextEventMatchingMask:NSEventMaskAny
                        untilDate:[NSDate distantPast]
                        inMode:NSDefaultRunLoopMode
                        dequeue:YES])) {
        // 키 코드 → KeyCode 매핑 람다 (macOS 물리 키코드 기준)
        auto toKeyCode = [](unsigned short kc) -> KeyCode {
            switch (kc) {
                case 13: return KeyCode::W;
                case  0: return KeyCode::A;
                case  1: return KeyCode::S;
                case  2: return KeyCode::D;
                case 15: return KeyCode::R;
                case 12: return KeyCode::Q;
                case 14: return KeyCode::E;
                case 49: return KeyCode::Space;
                case 56: return KeyCode::LeftShift;
                case 53: return KeyCode::Escape;
                case 48: return KeyCode::Tab;
                case 126: return KeyCode::Up;
                case 125: return KeyCode::Down;
                case 123: return KeyCode::Left;
                case 124: return KeyCode::Right;
                default:  return KeyCode::Count;  // 미지원 키
            }
        };

        if ([e type] == NSEventTypeKeyDown) {
            KeyCode kc = toKeyCode([e keyCode]);
            if (kc == KeyCode::Escape) { m_open = false; }
            else if (kc != KeyCode::Count) { InputManager::Get().OnKeyDown(kc); }
        } else if ([e type] == NSEventTypeKeyUp) {
            KeyCode kc = toKeyCode([e keyCode]);
            if (kc != KeyCode::Count) { InputManager::Get().OnKeyUp(kc); }
        } else if ([e type] == NSEventTypeLeftMouseDown) {
            InputManager::Get().OnMouseDown(MouseButton::Left);
        } else if ([e type] == NSEventTypeLeftMouseUp) {
            InputManager::Get().OnMouseUp(MouseButton::Left);
        } else if ([e type] == NSEventTypeRightMouseDown) {
            InputManager::Get().OnMouseDown(MouseButton::Right);
        } else if ([e type] == NSEventTypeRightMouseUp) {
            InputManager::Get().OnMouseUp(MouseButton::Right);
        } else if ([e type] == NSEventTypeMouseMoved
                || [e type] == NSEventTypeLeftMouseDragged
                || [e type] == NSEventTypeRightMouseDragged) {
            const NSPoint p = [e locationInWindow];
            const float scaleX = (m_width  > 0) ? static_cast<float>(m_pixelWidth)  / static_cast<float>(m_width)  : 1.0f;
            const float scaleY = (m_height > 0) ? static_cast<float>(m_pixelHeight) / static_cast<float>(m_height) : 1.0f;
            const int px = static_cast<int>(p.x * scaleX);
            const int py = static_cast<int>((static_cast<float>(m_height) - static_cast<float>(p.y)) * scaleY);
            InputManager::Get().OnMouseMove(px, py);
        } else if ([e type] == NSEventTypeScrollWheel) {
            InputManager::Get().OnMouseScroll(static_cast<float>([e scrollingDeltaY]));
        }
        [NSApp sendEvent:e];
    }

    auto now    = std::chrono::steady_clock::now();
    m_deltaTime = std::chrono::duration<float>(now - m_lastTime).count();
    m_lastTime  = now;

    // 이벤트 처리 중 Cocoa가 현재 컨텍스트를 바꿀 수 있으므로 렌더 직전에 다시 고정한다.
    [m_impl->context makeCurrentContext];
}

// OpenGL 백버퍼를 프론트버퍼로 교체해 화면에 표시
void MacWindow::SwapBuffers() {
    [m_impl->context makeCurrentContext];
    [m_impl->context flushBuffer];
}
