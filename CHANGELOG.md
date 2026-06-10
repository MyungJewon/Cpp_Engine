# Changelog

## [v0.3.0] - 2026-06-10

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

## [v0.2.0] - 이전 버전

- 구르기 물리 (접촉점 속도 기반 마찰 충격량)
- 절차적 텍스처 `Texture::FromPixels()`
- Angular velocity / drag
