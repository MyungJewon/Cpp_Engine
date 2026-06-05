cmake_minimum_required(VERSION 3.20)
project(Cpp_Engine)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_SOURCES
  src/main.cpp
  src/app/Application.cpp
  src/app/DemoApp.cpp
  src/app/GameLoop.cpp
  src/core/Time.cpp
  src/core/Path.cpp
  src/input/InputManager.cpp
  src/renderer/Framebuffer.cpp
  src/renderer/Rasterizer.cpp
  src/renderer/Pipeline.cpp
  src/renderer/ShadowMap.cpp
  src/renderer/ShadowPass.cpp
  src/renderer/OITBuffer.cpp
  src/renderer/Renderer.cpp
  src/renderer/gl/GLRenderer.cpp
  src/renderer/gl/GLShader.cpp
  src/renderer/gl/GLMesh.cpp
  src/renderer/shaders/PhongShader.cpp
  src/renderer/shaders/ShadowShader.cpp
  src/renderer/shaders/TransparentShader.cpp
  src/resource/AssetManager.cpp
  src/resource/MeshGenerator.cpp
  src/resource/ObjLoader.cpp
  src/resource/Texture.cpp
  src/scene/CameraController.cpp
  src/scene/Scene.cpp
  src/scene/Transform.cpp
  src/systems/InputSystem.cpp
  src/systems/ScriptSystem.cpp
  src/systems/TransformSystem.cpp
  src/systems/PhysicsSystem.cpp
  src/systems/CollisionSystem.cpp
  src/systems/CameraSystem.cpp
  src/systems/RenderSystem.cpp
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
target_include_directories(Cpp_Engine PRIVATE src)

if(APPLE)
  target_link_libraries(Cpp_Engine PRIVATE ${COCOA_LIB} ${APPKIT_LIB} ${OPENGL_LIB})
  set_source_files_properties(src/platform/MacWindow.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()

set_target_properties(Cpp_Engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin")

add_custom_command(TARGET Cpp_Engine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets"
          "$<TARGET_FILE_DIR:Cpp_Engine>/assets"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/model.obj"
          "$<TARGET_FILE_DIR:Cpp_Engine>/model.obj"
)
