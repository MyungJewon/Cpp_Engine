


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
│   ├── Material.h, MeshRenderer.h
│   └── (구) shaders/ — 소프트웨어 래스터라이저용 (현재 미사용)
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
컴포넌트 타입별로 `ComponentPool<T>` (`.hpp`)를 사용하며, 템플릿 특성상 헤더에 구현이 포함된다.

```cpp
// Entity 생성 + 컴포넌트 부착
auto& reg = m_scene.GetRegistry();
Entity e = m_scene.CreateEntity();
reg.add<Transform>(e, Transform{});
reg.add<MeshRenderer>(e, MeshRenderer{mesh, material});
reg.add<ScriptComponent>(e, {std::make_shared<RotatorScript>(40.f)});

// System 등록 (등록 순서 = 실행 순서)
m_world.add_system<InputSystem>();
m_world.add_system<ScriptSystem>();
m_world.add_system<TransformSystem>();
m_world.add_system<CameraSystem>(m_cameraController, m_scene);
m_world.add_system<RenderSystem>(m_renderer, m_scene, GetWindow());

// 매 프레임
m_world.update(dt);
```

### Transform (계층 구조)

쿼터니언 기반 회전. 부모가 변경되면 `MarkDirty()`가 자식 트리 전체에 재귀 전파되고,  
`GetWorldMatrix()`가 호출될 때만 실제 계산이 일어나는 지연 계산(lazy evaluation) 방식.

```cpp
// 부모-자식 설정 → 부모 이동 시 자식 자동 추종
childTf.SetParent(childEntity, parentEntity, registry);

// 회전 (쿼터니언 누적)
tf.SetLocalRot(tf.localRot * Quat::FromAxisAngle({0,1,0}, angle), registry);

// 월드 행렬 (dirty면 재계산, 아니면 캐시 반환)
Mat4 worldMat = tf.GetWorldMatrix(registry);
```

### 이벤트 버스

`std::type_index`를 키로 사용하는 타입 기반 pub/sub. 시스템 간 직접 참조 없이 통신.

```cpp
// 구독 (CameraSystem 생성자에서)
EventBus::Subscribe<CameraModeToggleEvent>([this](const CameraModeToggleEvent&) {
    m_controller.ToggleMode();
});

// 발행 (InputSystem에서 Tab 감지 시)
EventBus::Emit(CameraModeToggleEvent{});
```

### 스크립트 컴포넌트

Unity의 MonoBehaviour와 동일한 개념. `IScript`를 상속해 오브젝트별 동작을 정의한다.

```cpp
class MyScript : public IScript {
public:
    void OnUpdate(Entity self, Registry& reg, float dt) override {
        // 매 프레임 실행할 로직
    }
};

// Entity에 부착
reg.add<ScriptComponent>(e, {std::make_shared<MyScript>()});
```

### OpenGL 렌더러

렌더링은 두 패스로 진행된다: Shadow Pass → Opaque Pass.

- **Shadow Map** — 1024×1024 `GL_DEPTH_COMPONENT24` FBO, 광원 시점 깊이 기록
- **Phong 셰이딩** — GLSL, ambient / diffuse / specular, Normal Map (TBN) 지원
- **MSAA 4x** — `NSOpenGLPFAMultisample`, 폴백 포맷 자동 시도
- **Retina 대응** — `convertRectToBacking`으로 실제 픽셀 크기 획득 → `glViewport` 적용
- **지연 업로드** — CPU `Mesh` → GPU `VAO/VBO/EBO` 최초 렌더 시 한 번만 업로드
- **더미 텍스처** — albedo/normalMap 없을 때 1×1 흰색·평면법선 텍스처로 GPU 경고 방지

### AssetManager

경로를 키로 `unordered_map`에 캐싱. 같은 파일은 한 번만 로드한다.

```cpp
const Mesh*    mesh = AssetManager::Get().LoadMesh("model.obj");
const Texture* tex  = AssetManager::Get().LoadTexture("texture.png");
// 같은 경로로 두 번 호출해도 파일을 다시 읽지 않음
```

### MeshGenerator

OBJ 파일 없이 코드로 메시를 생성한다.

```cpp
Mesh grid = MeshGenerator::CreateGrid(20, 1.0f);  // 20×20 격자, 1유닛 간격
// 법선 (0,1,0), UV 좌표, (size+1)² 개 정점
```

---

## 데모 씬 구성

`DemoApp`이 엔진 사용 예시를 보여준다.

- 오브젝트 3개 (부모, 부모의 자식, 독립)
- 부모가 Y축 회전 → 자식이 함께 따라감 (계층 Transform 검증)
- 바닥 그리드 (MeshGenerator로 프로시저럴 생성)
- Orbit / FPS 카메라 전환
  
https://github.com/user-attachments/assets/d8b7417d-76ee-4253-bdec-aeb97bdb6c8a

---

## 개발 방식

이 프로젝트는 **Claude의 설계·추론**과 **Codex의 구현** 능력을 결합한 AI 협업 방식으로 제작되었습니다.

- 아키텍처 설계, 모듈 간 인터페이스 결정, 코드 리뷰 → **Claude (Anthropic)**
- 파일 생성, 실제 코드 구현, 빌드 오류 수정 → **Codex (OpenAI)**

설계와 구현을 분리하는 분업 구조를 통해 각 AI의 강점을 최대한 활용했습니다.
