cmake_minimum_required(VERSION 3.20)
project(Cpp_Engine)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_SOURCES
  src/main.cpp
  src/app/Application.cpp
  src/app/DemoApp.cpp
  src/app/GameLoop.cpp
  src/audio/AudioManager.cpp
  src/core/Time.cpp
  src/core/Path.cpp
  src/input/InputManager.cpp
  src/physics/Raycast.cpp
  src/renderer/ShadowMap.cpp
  src/renderer/Renderer.cpp
  src/renderer/Skybox.cpp
  src/renderer/gl/GLRenderer.cpp
  src/renderer/gl/GLShader.cpp
  src/renderer/gl/GLMesh.cpp
  src/ui/UIRenderer.cpp
  src/resource/AssetManager.cpp
  src/resource/MeshGenerator.cpp
  src/resource/ObjLoader.cpp
  src/resource/Texture.cpp
  src/scene/CameraController.cpp
  src/scene/Scene.cpp
  src/scene/SceneSerializer.cpp
  src/scene/Transform.cpp
  src/systems/InputSystem.cpp
  src/systems/ScriptSystem.cpp
  src/systems/TransformSystem.cpp
  src/systems/PhysicsSystem.cpp
  src/systems/CollisionSystem.cpp
  src/systems/CameraSystem.cpp
  src/systems/RenderSystem.cpp
  src/systems/AudioSystem.cpp
  src/systems/UISystem.cpp
)

if(APPLE)
  enable_language(OBJCXX)
  list(APPEND ENGINE_SOURCES src/platform/MacWindow.mm)
  find_library(COCOA_LIB Cocoa REQUIRED)
  find_library(APPKIT_LIB AppKit)
  find_library(OPENGL_LIB OpenGL REQUIRED)
else()
  list(APPEND ENGINE_SOURCES src/platform/Win32Window.cpp)
endif()

add_executable(Cpp_Engine ${ENGINE_SOURCES})
target_include_directories(Cpp_Engine PRIVATE . src vendor)

if(APPLE)
  target_link_libraries(Cpp_Engine PRIVATE ${COCOA_LIB} ${APPKIT_LIB} ${OPENGL_LIB})
  set_source_files_properties(src/platform/MacWindow.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()

set_target_properties(Cpp_Engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin")

add_custom_target(copy_assets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets"
          "${CMAKE_SOURCE_DIR}/build/bin/assets"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/model.obj"
          "${CMAKE_SOURCE_DIR}/build/bin/model.obj"
  COMMENT "Copying assets to build directory"
)
add_dependencies(copy_assets Cpp_Engine)
