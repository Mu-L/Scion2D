# SCION_UTILITIES

General-purpose utilities shared across all Scion2D modules.

## What It Contains

- `HelperUtilities.h` -- path separator constant, base path macro, project path macro,
  and `SpriteLayerParams` struct.
- `MathUtilities.h` -- `PI`, `TwoPI`, `PIOver2`, `PIOver4` constants.
- `ScionUtilities.h` -- `AssetType` enum, `S2DAsset` struct, and template map utilities
  (`GetKeys`, `CheckContainsValue`, `KeyChange`).
- `RandomGenerator` -- `RandomIntGenerator` and `RandomFloatGenerator` backed by `std::mt19937`.
- `Timer` -- simple `steady_clock` timer with start, stop, pause, resume, and elapsed query.
- `Tween` -- single-value tween with 19 easing functions (linear, quad, sine, elastic,
  exponential, bounce, circ).
- `ThreadPool` -- standard fixed-size thread pool using a `std::queue<function<void()>>` and
  `std::condition_variable`. Returns `std::future` from `Enqueue`.
- `SDL_Wrappers` -- RAII helpers for SDL types (`SDL_Window`, `SDL_Cursor`, etc.) using
  shared_ptr with custom deleters.

## Dependencies
`SDL3`, `std` (C++23)
