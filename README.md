# Cpp_Engine

외부 라이브러리 없이 C++17만으로 제작한 미니 3D 게임 엔진.
OpenGL 4.1 기반 렌더러, Sparse Set ECS, 계층 Transform, 이벤트 시스템 등을 포함한다.

---

## 특징

- **외부 의존성 없음** — CMake + C++17 + OpenGL (macOS 기본 제공) 만으로 빌드
- **ECS 아키텍처** — Entity / Component / System 구조로 게임 오브젝트 관리
- **OpenGL 4.1 렌더러** — GPU 기반 Phong 셰이딩, Shadow Map, MSAA
- **계층 Transform** — 부모-자식 관계, dirty 플래그 기반 월드 행렬 캐싱
- **이벤트 버스** — 타입 기반 pub/sub, 시스템 간 직접 결합 없이 통신
- **스크립트 컴포넌트** — `IScript` 상속으로 오브젝트별 동작 정의
- **Orbit / FPS 카메라** — Tab으로 전환, 마우스 드래그·휠 조작

---

## 빌드 방법

```bash
# 처음 한 번만
cmake -S . -B build

# 빌드
cmake --build build

# 실행
./build/bin/Cpp_Engine
```

> macOS 전용. Xcode Command Line Tools 필요.

---

## 조작

| 입력 | 동작 |
|------|------|
| **Tab** | Orbit ↔ FPS 카메라 전환 |
| **Orbit** 좌클릭 드래그 | 오브젝트 중심 회전 |
| **Orbit** 스크롤 | 줌 인/아웃 |
| **FPS** 우클릭 드래그 | 시선 방향 회전 |
| **FPS** W / A / S / D | 앞 / 왼 / 뒤 / 오른 이동 |
| **FPS** Q / E | 아래 / 위 이동 |
| **ESC** | 종료 |

---

## 디렉토리 구조

```
src/
├── app/            Application, GameLoop (엔진 진입점 및 루프)
├── core/           Time, Path (프레임 타이밍, 경로 유틸)
├── ecs/            Entity, ComponentPool, Registry, View, World, System (ECS 코어)
├── event/          EventBus, Events (타입 기반 pub/sub)
├── input/          InputManager, InputCodes (키/마우스 상태)
├── math/           Vec2/3/4, Mat4, Quat, MathUtils (수학 라이브러리)
├── platform/       IWindow, MacWindow, Win32Window (플랫폼 추상화)
├── renderer/
│   ├── gl/         GLRenderer, GLShader, GLMesh (OpenGL 렌더러)
│   └── shaders/    PhongShader, ShadowShader (소프트웨어 래스터라이저용 셰이더)
├── resource/       AssetManager, ObjLoader, Texture, MeshGenerator
├── scene/          Scene, Transform, Camera, Light, CameraController
├── script/         IScript, ScriptComponent, RotatorScript
├── systems/        InputSystem, TransformSystem, CameraSystem, RenderSystem, ScriptSystem
└── main.cpp

assets/
└── shaders/        phong.vert/frag, shadow.vert/frag (GLSL 셰이더)
```

---

## 주요 모듈

### ECS (Entity Component System)

Sparse Set 기반으로 구현. O(1) 삽입/삭제, 캐시 친화적 순회.

```cpp
// Entity 생성 + 컴포넌트 부착
Entity e = scene.CreateEntity();
scene.GetRegistry().add<Transform>(e, Transform{});
scene.GetRegistry().add<MeshRenderer>(e, MeshRenderer{mesh, material});
scene.GetRegistry().add<ScriptComponent>(e, {std::make_shared<RotatorScript>(40.f)});

// System 등록
world.add_system<ScriptSystem>();
world.add_system<TransformSystem>();
world.add_system<RenderSystem>(renderer, scene, window);

// 루프
world.update(dt);
```

### Transform (계층 구조)

```cpp
// 부모-자식 설정 → 부모 이동 시 자식 자동 추종
tf.SetParent(child, parent, registry);

// 회전 (쿼터니언)
tf.SetLocalRot(Quat::FromAxisAngle({0,1,0}, angle), registry);

// 월드 행렬 (dirty 캐싱)
Mat4 world = tf.GetWorldMatrix(registry);
```

### 이벤트 버스

```cpp
// 구독
EventBus::Subscribe<CameraModeToggleEvent>([this](const CameraModeToggleEvent&) {
    // 처리
});

// 발행
EventBus::Emit(CameraModeToggleEvent{});
```

### 스크립트 컴포넌트

```cpp
class MyScript : public IScript {
public:
    void OnUpdate(Entity self, Registry& reg, float dt) override {
        // 매 프레임 실행할 로직
    }
};

// Entity에 부착
registry.add<ScriptComponent>(e, {std::make_shared<MyScript>()});
```

### OpenGL 렌더러

- **Shadow Map** — 1024×1024 depth FBO, slope-scale bias
- **Phong 셰이딩** — GLSL, diffuse / specular / ambient, Normal Map 지원
- **MSAA 4x** — `NSOpenGLPFAMultisample`
- **Retina 대응** — backing store 픽셀 크기로 `glViewport` 설정
- **메시/텍스처 캐싱** — CPU Mesh → GPU VAO/VBO 지연 업로드

### AssetManager

```cpp
// 경로 기반 캐싱 — 같은 경로는 한 번만 로드
const Mesh*    mesh = AssetManager::Get().LoadMesh("model.obj");
const Texture* tex  = AssetManager::Get().LoadTexture("texture.tga");
```

### MeshGenerator

```cpp
// 코드로 메시 생성 (OBJ 파일 불필요)
Mesh grid = MeshGenerator::CreateGrid(20, 1.0f);  // 20×20 격자, 1유닛 간격
```

---

## 데모 씬 구성

`DemoApp`이 엔진 사용 예시를 보여준다.

- 오브젝트 3개 (부모, 부모의 자식, 독립)
- 부모가 Y축 회전 → 자식이 함께 따라감 (계층 Transform 검증)
- 바닥 그리드 (MeshGenerator로 프로시저럴 생성)
- Orbit / FPS 카메라 전환

---

## 개발 방식

이 프로젝트는 **Claude의 설계·추론**과 **Codex의 구현** 능력을 결합한 AI 협업 방식으로 제작되었습니다.

- 아키텍처 설계, 모듈 간 인터페이스 결정, 코드 리뷰 → **Claude (Anthropic)**
- 파일 생성, 실제 코드 구현, 빌드 오류 수정 → **Codex (OpenAI)**

설계와 구현을 분리하는 분업 구조를 통해 각 AI의 강점을 최대한 활용했습니다.
