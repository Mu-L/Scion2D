# SCION_CORE

The central library of Scion2D. All other modules depend on or integrate with SCION_CORE.
It owns the ECS registry wrapper, the scene and scene manager, all component types, all engine
systems, the asset manager, the scripting layer, and the event dispatcher.

## What It Contains

### ECS
- `Registry` -- wrapper around `entt::registry`. Adds context storage, pending destruction queues, and Lua binding helpers.
- `MainRegistry` -- singleton that owns the global registry, asset manager, audio player, renderer, and all engine systems.
- `Entity` -- thin wrapper that holds an `entt::entity` and a registry pointer. Used for Lua bindings.
- `MetaUtilities` -- helpers for invoking entt meta functions by type id. Used for dynamic component access from Lua.

### Components
All serializable game object components live here. Key ones:
- `TransformComponent` -- world position, local position, scale, rotation.
- `SpriteComponent` -- texture name, UV rect, color, layer, iso flags.
- `AnimationComponent` -- frame count, frame rate, loop flag, current frame.
- `PhysicsComponent` -- holds the Box2D body pointer.
- `BoxColliderComponent`, `CircleColliderComponent` -- collider size and offset.
- `ScriptComponent` -- Lua script path and update/render table references.
- `Identification` -- entity name, group, and tag.
- `Relationship` -- parent/child hierarchy links.
- `UIComponent`, `TextComponent`, `TileComponent`, `RigidBodyComponent` -- supporting component types.

### Systems
- `AnimationSystem` -- advances sprite UV frames based on SDL ticks and frame rate.
- `PhysicsSystem` -- syncs Box2D body positions back into `TransformComponent` each frame.
- `RenderSystem` -- submits sprites to the `SpriteBatchRenderer`.
- `RenderUISystem` -- submits UI sprites using the UI camera.
- `RenderShapeSystem` -- submits debug shapes (rects, circles, lines).
- `RenderPickingSystem` -- renders entity IDs into the picking framebuffer for mouse-over selection in the editor.
- `ScriptingSystem` -- calls Lua update/render functions on entities with `ScriptComponent`.

### Resources
- `AssetManager` -- loads and caches textures, fonts, shaders, audio, and prefabs. Supports hot-reload via a background file watcher thread.

### Scripting
Lua binding helpers for all components, the registry, the asset manager, the event dispatcher, physics contacts, sound, and filesystem access.

### Events
`EventDispatcher` -- wrapper around `entt::dispatcher`. Supports typed event emission, queuing, and Lua-accessible handlers.

### Scene
- `Scene` -- owns a registry, canvas, layer list, player start, and load/save logic.
- `SceneManager` -- manages the set of named scenes; handles scene switching.

### Profiling
- `ProfileCollector` -- in-editor ring buffer profiler. Receives zone data from dual-emission Tracy macros and stores it for the ImGui profiler display.

## Dependencies
`entt`, `sol2`, `box2d`, `glm`, `SDL3`, `SCION_RENDERING`, `SCION_PHYSICS`, `SCION_SOUNDS`, `SCION_LOGGER`, `SCION_UTILITIES`, `SCION_FILESYSTEM`
