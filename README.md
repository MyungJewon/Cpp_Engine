
# Cpp_Engine

외부 라이브러리 없이 C++17만으로 제작한 미니 3D 게임 엔진.
OpenGL 4.1 기반 렌더러, Sparse Set ECS, 계층 Transform, 강체 물리 시스템 등을 포함한다.

---

## 특징

- **외부 의존성 없음** — CMake + C++17 + OpenGL (macOS 기본 제공) 만으로 빌드
- **ECS 아키텍처** — Sparse Set 기반 Entity / Component / System, O(1) 삽입·삭제
- **OpenGL 4.1 렌더러** — GPU 기반 Phong 셰이딩, PCF 소프트 Shadow Map, MSAA 4x
- **계층 Transform** — 부모-자식 관계, dirty 플래그 기반 월드 행렬 지연 계산
- **강체 물리** — 선속도·각속도 적분, AABB/Sphere 충돌, 충격량 기반 마찰·반발
- **구르기 물리** — 접촉점 속도 `v + cross(ω, r)` 기준 마찰 충격 → 선·각속도 동시 갱신
- **이벤트 버스** — 타입 기반 pub/sub, 시스템 간 직접 결합 없이 통신
- **스크립트 컴포넌트** — `IScript` 상속으로 오브젝트별 동작 정의
- **Orbit / FPS 카메라** — Tab으로 전환, 마우스 드래그·휠 조작
- **절차적 텍스처** — `Texture::FromPixels()`로 런타임 픽셀 데이터에서 텍스처 생성
- **오디오 시스템** — `AudioSource` 컴포넌트, `AudioManager` 싱글톤 (miniaudio 기반, WAV/MP3, PlayOneShot 겹침 재생, BGM 루프)
- **씬 직렬화** — `SceneSerializer`로 씬을 JSON 파일로 저장·불러오기 (nlohmann/json)
- **씬 전환** — `IScene` 인터페이스와 `Application::LoadScene()`으로 씬 간 전환
- **UI 렌더링** — `UIComponent` + `UISystem` + `UIRenderer` (2D 오버레이, TrueType 한글·영문 폰트, 클릭 감지, 화면 좌표계)

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
├── app/            Application, GameLoop, IScene (엔진 진입점·씬 전환)
├── audio/          AudioClip, AudioSource, AudioManager
├── core/           Time, Path, Color (프레임 타이밍, 경로 유틸, 색상 타입)
├── ecs/            Entity, ComponentPool, Registry, View, World, System
├── event/          EventBus, Events (타입 기반 pub/sub)
├── input/          InputManager, InputCodes (키/마우스 상태)
├── math/           Vec2/3/4, Mat4, Quat, MathUtils
├── physics/        RigidBody, Collider
├── platform/       IWindow, MacWindow (플랫폼 추상화)
├── renderer/
│   ├── gl/         GLRenderer, GLShader, GLMesh (OpenGL 렌더러)
│   └── Material.h, MeshRenderer.h
├── resource/       AssetManager, ObjLoader, Texture, MeshGenerator
├── scene/          Scene, Transform, Camera, Light, CameraController, SceneSerializer
├── script/         IScript, ScriptComponent, RotatorScript
├── systems/        InputSystem, TransformSystem, CameraSystem,
│                   RenderSystem, ScriptSystem, PhysicsSystem, CollisionSystem,
│                   AudioSystem, UISystem
├── ui/             UIComponent, UIRenderer
└── main.cpp

assets/
├── audio/
├── scenes/
└── shaders/        phong.vert/frag, shadow.vert/frag (GLSL)

vendor/
└── miniaudio.h, nlohmann/json.hpp (서드파티 헤더)
```

---

## 주요 모듈

### ECS (Entity Component System)

Sparse Set 기반으로 구현. O(1) 삽입/삭제, 캐시 친화적 순회.
템플릿 클래스(`ComponentPool<T>`, `Registry`, `World`)는 `.hpp` 헤더에 구현이 포함된다.

```cpp
auto& reg = m_scene.GetRegistry();
Entity e = m_scene.CreateEntity();
reg.add<Transform>(e, Transform{});
reg.add<MeshRenderer>(e, MeshRenderer{mesh, material});
reg.add<RigidBody>(e, RigidBody{});
reg.add<Collider>(e, Collider::FromMesh(*mesh, transform.localScale));

// System 등록 (등록 순서 = 실행 순서)
m_world.add_system<InputSystem>();
m_world.add_system<ScriptSystem>();
m_world.add_system<TransformSystem>();
m_world.add_system<CameraSystem>(m_cameraController, m_scene);
m_world.add_system<RenderSystem>(m_renderer, m_scene, GetWindow());

// 물리 시스템은 FixedUpdate (1/60초 고정)
m_world.add_fixed_system<PhysicsSystem>();
m_world.add_fixed_system<CollisionSystem>();

m_world.update(dt);        // 매 프레임
m_world.fixed_update(dt);  // OnFixedUpdate에서
```

### Transform (계층 구조)

쿼터니언 기반 회전. `MarkDirty()`가 자식 트리 전체에 재귀 전파되고,
`GetWorldMatrix()`가 호출될 때만 실제 계산이 일어나는 지연 계산 방식.

```cpp
// 부모-자식 설정
childTf.SetParent(childEntity, parentEntity, registry);

// 쿼터니언 회전 누적
tf.SetLocalRot(tf.localRot * Quat::FromAxisAngle({0,1,0}, angle), registry);

// 월드 행렬 (dirty면 재계산, 아니면 캐시 반환)
Mat4 worldMat = tf.GetWorldMatrix(registry);
```

### 물리 시스템

`PhysicsSystem`과 `CollisionSystem`이 `FixedUpdate(1/60초)`에서 실행된다.

```cpp
// RigidBody — 중력, 선속도, 각속도
struct RigidBody {
    Vec3  velocity        = {0, 0, 0};
    Vec3  angularVelocity = {0, 0, 0};  // rad/s
    float mass            = 1.0f;
    float drag            = 0.01f;
    float angularDrag     = 1.5f;       // 회전 감쇠
    bool  useGravity      = true;
    bool  isKinematic     = false;
};

// Collider — 충돌 형태 및 물성
struct Collider {
    ColliderShape shape       = ColliderShape::AABB;
    Vec3          halfExtents = {0.5f, 0.5f, 0.5f};
    float         radius      = 0.5f;
    float         restitution = 0.0f;  // 반발력 (0~1)
    float         friction    = 0.4f;  // 마찰력 (0~1)
    bool          isTrigger   = false;

    // 메시 크기에서 AABB 자동 계산 (scale bake-in)
    static Collider FromMesh(const Mesh& mesh, const Vec3& scale = {1,1,1});
};
```

충돌 응답은 충격량(impulse) 기반으로 계산된다.
- **구르기 물리**: 접촉점 속도 `v + cross(ω, r)` 기준으로 마찰 충격량을 계산하고, 선속도와 각속도에 동시 적용. 유효 질량(effective mass)으로 Coulomb 마찰 한계 산출.
- RigidBody 없는 Entity는 static(고정) 오브젝트로 처리
- Trigger이면 `TriggerEnterEvent` 발행, 일반 충돌이면 위치 보정 + 속도 반사

### 이벤트 버스

`std::type_index`를 키로 사용하는 타입 기반 pub/sub.

```cpp
// 구독
EventBus::Subscribe<CameraModeToggleEvent>([this](const CameraModeToggleEvent&) {
    m_controller.ToggleMode();
});

// 발행
EventBus::Emit(CameraModeToggleEvent{});
EventBus::Emit(CollisionEvent{entityA, entityB});
```

### 스크립트 컴포넌트

Unity의 MonoBehaviour와 동일한 개념.

```cpp
class MyScript : public IScript {
public:
    void OnInit(Entity self, Registry& reg) override { /* 초기화 */ }
    void OnUpdate(Entity self, Registry& reg, float dt) override { /* 매 프레임 */ }
};

reg.add<ScriptComponent>(e, {std::make_shared<MyScript>()});
```

### OpenGL 렌더러

Shadow Pass → Opaque Pass 두 패스로 렌더링.

- **Shadow Map** — 1024×1024 depth FBO, slope-scale bias
- **PCF 필터링** — 3×3 샘플 평균으로 그림자 경계 부드럽게 처리
- **Phong 셰이딩** — ambient / diffuse / specular, Normal Map (TBN) 지원
- **MSAA 4x** — `NSOpenGLPFAMultisample`, 폴백 포맷 자동 시도
- **Retina 대응** — `convertRectToBacking`으로 실제 픽셀 크기 획득
- **지연 업로드** — CPU `Mesh` → GPU `VAO/VBO/EBO` 최초 렌더 시 한 번만 업로드

### AssetManager

경로를 키로 캐싱. 같은 파일은 한 번만 로드한다.

```cpp
const Mesh*    mesh = AssetManager::Get().LoadMesh("model.obj");
const Texture* tex  = AssetManager::Get().LoadTexture("texture.png");
```

### MeshGenerator

OBJ 파일 없이 코드로 메시를 생성한다.

```cpp
Mesh grid   = MeshGenerator::CreateGrid(20, 1.0f);        // 20×20 격자
Mesh sphere = MeshGenerator::CreateSphere(16, 16, 1.0f);  // UV 구체
```

### Texture

TGA 파일 로드 또는 런타임 픽셀 데이터로 텍스처를 생성한다.

```cpp
// 파일에서 로드
const Texture* tex = AssetManager::Get().LoadTexture("texture.tga");

// 코드에서 생성 (절차적 텍스처)
std::vector<Color> pixels(8 * 8);
for (int y = 0; y < 8; y++)
    for (int x = 0; x < 8; x++)
        pixels[y * 8 + x] = ((x + y) % 2 == 0)
            ? Color(255, 255, 255, 255)
            : Color(220, 50, 50, 255);
Texture checker = Texture::FromPixels(8, 8, pixels);
material.albedo = &checker;  // 멤버 변수로 수명 관리 필요
```

### 오디오 시스템

```cpp
// 초기화
AudioManager::Get().Init();

// 단발 효과음 (겹침 재생 가능)
AudioClip hitClip;
hitClip.path = "assets/audio/hit.wav";
AudioManager::Get().PlayOneShot(&hitClip, 1.0f);

// BGM 루프
AudioClip bgm;
bgm.path = "assets/audio/bgm.mp3";
AudioManager::Get().Play(&bgm, 0.5f, true);

// 충돌 이벤트 연동
EventBus::Subscribe<CollisionEvent>([&](const CollisionEvent&) {
    AudioManager::Get().PlayOneShot(&hitClip);
});
```

### 씬 직렬화

```cpp
// 저장
SceneSerializer::Save(m_scene, "assets/scenes/level1.scene");

// 불러오기
SceneSerializer::Load("assets/scenes/level1.scene", m_scene);
// MeshRenderer::mesh 포인터는 로드 후 meshName으로 직접 연결
for (auto [e, mr] : reg.view<MeshRenderer>()) {
    if (mr.meshName == "sphere") mr.mesh = &m_sphereMesh;
}
```

### 씬 전환

```cpp
class TitleScene : public IScene {
    void OnEnter() override { /* UI 초기화 */ }
    void OnUpdate(float dt) override {
        if (/* 시작 버튼 */)
            m_app->LoadScene(std::make_unique<GameScene>());
    }
    void OnRender() override { /* 렌더링 */ }
};

// 앱에서 초기 씬 설정
app.LoadScene(std::make_unique<TitleScene>());
app.Run();
```

### UI 시스템

2D 오버레이 UI. `UIComponent`를 ECS Entity에 추가해 사용한다.

```cpp
// 텍스트
Entity textEntity = m_scene.CreateEntity();
UIComponent text;
text.type     = UIType::Text;
text.text     = "안녕하세요 Hello";
text.x        = 10.0f;
text.y        = 10.0f;
text.fontSize = 20;          // px 단위
text.color    = { 1, 1, 1 };
m_scene.GetRegistry().add<UIComponent>(textEntity, text);

// 클릭 가능한 버튼
Entity btnEntity = m_scene.CreateEntity();
UIComponent btn;
btn.type    = UIType::Rect;
btn.x       = 10.0f;
btn.y       = 50.0f;
btn.width   = 120.0f;
btn.height  = 40.0f;
btn.color   = { 0.2f, 0.6f, 1.0f };
btn.onClick = []() { /* 클릭 시 실행 */ };
m_scene.GetRegistry().add<UIComponent>(btnEntity, btn);
```

클릭 판정은 `(x, y, width, height)` AABB 기준. 텍스트에도 `width/height`를 설정하면 클릭 가능.

---

## 데모 씬 구성

`DemoApp`이 엔진 사용 예시를 보여준다.

- 구체 2개가 중력으로 낙하 → 바닥과 충돌 후 구르기
- 구체 간 충돌 감지 및 반발·마찰·회전 적용
- 체커보드 텍스처로 회전 시각화
- Orbit / FPS 카메라 전환


https://github.com/user-attachments/assets/4cb8b495-0670-441d-80b1-6110b2eb0a60

https://github.com/user-attachments/assets/d8b7417d-76ee-4253-bdec-aeb97bdb6c8a

---

## 개발 방식

이 프로젝트는 **Claude의 설계·추론**과 **Codex의 구현** 능력을 결합한 AI 협업 방식으로 제작되었습니다.

- 아키텍처 설계, 모듈 간 인터페이스 결정, 코드 리뷰 → **Claude (Anthropic)**
- 파일 생성, 실제 코드 구현, 빌드 오류 수정 → **Codex (OpenAI)**

설계와 구현을 분리하는 분업 구조를 통해 각 AI의 강점을 최대한 활용했습니다.
