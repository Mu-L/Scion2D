# SCION_ENGINE

The runtime executable that is shipped with a packaged game. It contains no editor code.
When a project is exported, the game assets and Lua scripts are bundled alongside this executable.

## What It Contains

- `Runtime` -- the main game loop class. Initializes the engine, loads the main Lua script,
  runs the scene stack, and shuts everything down cleanly.
- `main.cpp` -- entry point. Creates a `Runtime` instance and starts the loop.

## How It Differs From the Editor

The engine build defines no `IN_SCION_EDITOR` preprocessor flag. This strips all ImGui,
editor display, and editor-only asset manager code from the build, leaving a lean runtime
with only what the game needs.

## Dependencies
`SCION_CORE`, `SCION_RENDERING`, `SCION_PHYSICS`, `SCION_SOUNDS`, `SCION_LOGGER`, `SCION_UTILITIES`, `SCION_WINDOW`, `SDL3`, `OpenGL 4.6`, `sol2`, `entt`
