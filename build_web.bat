@echo off
emcc -std=c++14 -O2 ^
  -s USE_GLFW=3 ^
  -s FULL_ES3=1 ^
  -s USE_FREETYPE=1 ^
  -s ALLOW_MEMORY_GROWTH=1 ^
  -s TOTAL_MEMORY=256MB ^
  -DPLATFORM_WEB ^
  --preload-file assets ^
  --preload-file shaders ^
  --preload-file map ^
  -IExis/src ^
  -IExis/external ^
  -IExis/external/glm ^
  Exis/src/application/application.cpp ^
  Exis/src/core/arena.cpp ^
  Exis/src/core/audioengineweb.cpp ^
  Exis/src/core/camera.cpp ^
  Exis/src/core/ecs.cpp ^
  Exis/src/core/engine.cpp ^
  Exis/src/core/input.cpp ^
  Exis/src/core/profiler.cpp ^
  Exis/src/core/serialization.cpp ^
  Exis/src/core/tilemap.cpp ^
  Exis/src/core/tracelog.cpp ^
  Exis/src/core/ui.cpp ^
  Exis/src/core/animationmanager.cpp ^
  Exis/src/core/colliders.cpp ^
  Exis/src/core/mystring.cpp ^
  Exis/src/renderer/fontmanager.cpp ^
  Exis/src/renderer/renderer.cpp ^
  Exis/src/renderer/shader.cpp ^
  Exis/src/renderer/texture.cpp ^
  Exis/src/platform/platformweb.cpp ^
  Exis/src/platform/applicationweb.cpp ^
  game/projectx.cpp ^
  game/fluidsimulator.cpp ^
  -o game.html

echo Build complete! Run: python -m http.server 8080
