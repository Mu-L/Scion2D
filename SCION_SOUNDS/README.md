# SCION_SOUNDS

Audio playback layer for Scion2D, built on SDL3_mixer.

## What It Contains

- `Audio` -- wraps an `MIX_Audio*` handle and stores the audio type (Music or SoundFX) and
  the source filepath.
- `AudioPlayer` -- controls playback. Wraps the SDL3_mixer channel/stream API for playing,
  pausing, stopping, and volume control for both music tracks and sound effects.
- `Utilities` -- audio helper functions shared internally.

## Audio Types
- `AudioType::Music` -- long-form background music. Streamed.
- `AudioType::Soundfx` -- short sound effects. Loaded fully into memory.

## Lua Access
Audio is accessible from Lua via the `AssetManager` and via `SoundBindings` which expose
play, stop, pause, resume, and volume functions.

## Dependencies
`SDL3_mixer`, `SCION_LOGGER`
