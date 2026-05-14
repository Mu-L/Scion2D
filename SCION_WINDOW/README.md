# SCION_WINDOW

SDL3 window and input management for Scion2D.

## What It Contains

### Window
- `Window` -- creates and owns the SDL3 window and OpenGL context. Exposes swap, resize,
  and event polling. Wraps the SDL3 window lifetime in RAII.

### Input
- `Keyboard` -- per-frame keyboard state. Wraps SDL3 scancode queries. Exposes `IsKeyPressed`,
  `IsKeyHeld`, and `IsKeyReleased`.
- `Mouse` -- per-frame mouse state. Position (screen and world), button pressed/held/released,
  and scroll delta.
- `Gamepad` -- SDL3 gamepad input. Button and axis state for a single connected gamepad.
- `Button` -- shared base state struct for button-style inputs (pressed, held, released this frame).
- `Keys.h` -- `EKey` enum mapping Scion key names to SDL3 scancodes.
- `MouseButtons.h` -- `EMouseButton` enum.
- `GPButtons.h` -- `EGPButton` enum for gamepad buttons.

## Dependencies
`SDL3`, `OpenGL 4.6` (context creation), `SCION_LOGGER`
