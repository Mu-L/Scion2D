# SCION_PHYSICS

Physics integration layer for Scion2D, wrapping Box2D 2.4.2.

## What It Contains

- `Box2DWrappers` -- `b2World` and `b2Body` wrapped in RAII types so they integrate cleanly
  with the engine's ownership model.
- `ContactListener` -- implements `b2ContactListener`. Fires `BeginContact` and `EndContact`
  callbacks and populates `ObjectData` contact sets used by the Lua physics event system.
- `UserData` -- the `ObjectData` struct stored in each Box2D fixture's user data pointer.
  Holds the entity ID and a contact list for the current frame.
- `PhysicsUtilities` -- helpers for creating bodies and fixtures from `PhysicsComponent` data.
- `RayCastCallback` / `BoxTraceCallback` -- implement `b2RayCastCallback` and AABB query
  callbacks. Used for ray casts and box traces from Lua.

## Box2D Version Note
This module uses Box2D **2.4.2**, not the current 3.x release. Box2D 3.x uses a C API that
is not compatible with the wrapper design used here. The CMakeLists fetches the correct version
via FetchContent. Do not upgrade vcpkg box2d to 3.x without a corresponding migration of all
wrappers.

## Dependencies
`box2d 2.4.2`, `entt`, `SCION_LOGGER`
