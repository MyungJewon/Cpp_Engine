# Changelog

## [v0.4.0] - 2026-06-10

### Added
- UI TrueType 폰트 지원: `stb_truetype` 기반 한글·영문 렌더링 (4096×2048 아틀라스, AppleSDGothicNeo)
- UI 클릭 감지: `UIComponent.onClick` 콜백, `UISystem`에서 AABB hit test
- `InputManager::JustMousePressed(MouseButton)` — 마우스 버튼 단일 프레임 감지

### Fixed
- `UIComponent.fontSize`가 배율이 아닌 px 단위로 동작하도록 수정 (`fontSize=20` → 20px)
- macOS 마우스 좌표 Cocoa Y축 반전 및 Retina 스케일 보정 (UI 클릭 위치 정확도 수정)

## [v0.3.0] - 2026-06-10

### Added
- 레이캐스트 시스템: `Raycast::Cast()`, `Raycast::ScreenPointToRay()` — Ray-Sphere / Ray-AABB 교차 검출
- 스카이박스: Equirectangular 파노라마 PNG 렌더링 (`Skybox`, `skybox.vert/frag`)
- stb_image 통합 — 텍스처 로더가 PNG/JPG 등 범용 포맷 지원
- 충돌 이벤트 분리: `CollisionEnterEvent` / `CollisionStayEvent` / `CollisionExitEvent`

### Changed
- `AudioManager::PlayOneShot` — 콜백 방식 제거, active sound 목록으로 안전한 종료 처리
- CMake 빌드 시 셰이더·에셋이 항상 `build/bin`에 복사되도록 개선

## [v0.2.0] - 2026-06-10

### Added
- 오디오 시스템: `AudioClip`, `AudioSource`, `AudioManager` (miniaudio), `AudioSystem`
  - WAV/MP3 재생, PlayOneShot 겹침 재생, BGM 루프, 마스터 볼륨
- 씬 직렬화: `SceneSerializer` — JSON 기반 씬 저장·불러오기 (nlohmann/json)
  - 직렬화 대상: Transform, RigidBody, Collider, MeshRenderer, AudioSource, UIComponent
- 씬 전환: `IScene` 인터페이스, `Application::LoadScene()`
- UI 렌더링: `UIComponent`, `UIRenderer`, `UISystem` — 화면 좌표계 2D 오버레이, 비트맵 폰트
- `Color` 구조체를 `src/core/Color.h`로 분리

### Changed
- `MeshRenderer`에 `meshName` 필드 추가 — 직렬화 시 메쉬 식별
- GLShader uniform 위치 캐싱으로 렌더 성능 개선
- UIRenderer uniform 위치 캐싱

### Removed
- 소프트웨어 렌더러 제거: `Framebuffer`, `Rasterizer`, `Pipeline`, `ShadowPass`, `OITBuffer`, `PhongShader`, `ShadowShader`, `TransparentShader`

## [v0.1.1] - 이전 버전

- 구르기 물리 (접촉점 속도 기반 마찰 충격량)
- 절차적 텍스처 `Texture::FromPixels()`
- Angular velocity / drag
