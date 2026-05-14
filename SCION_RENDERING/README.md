# SCION_RENDERING

OpenGL 4.6 rendering layer for Scion2D. Handles all GPU resource management and batch rendering.

## What It Contains

### Core
- `SpriteBatchRenderer` -- sorts sprites by layer and texture, builds batches, and issues draw calls.
  Inherits from `Batcher<Batch, SpriteGlyph>`.
- `TextBatchRenderer` -- renders glyph quads for `TextComponent` entities.
- `CircleBatchRenderer` -- renders anti-aliased circles via a GLSL SDF shader.
- `RectBatchRenderer` -- renders axis-aligned rectangles.
- `LineBatchRenderer` -- renders debug lines.
- `PickingBatchRenderer` -- renders entity IDs into an integer framebuffer for mouse picking.
- `Batcher<TBatch, TGlyph>` -- base template managing VAO/VBO/IBO setup, flush, and the glyph/batch
  vectors used by all renderers.
- `Camera2D` -- orthographic camera. Maintains view and projection matrices.
- `Renderer` -- top-level renderer. Owns all batch renderers and the camera.

### Essentials
- `Texture` / `TextureLoader` -- loads image files or memory buffers into OpenGL textures. Supports
  pixel art (nearest-neighbor) and blended (linear) filter modes.
- `Shader` / `ShaderLoader` -- compiles and links GLSL programs. Caches uniform locations.
- `Font` / `FontLoader` -- loads TTF fonts via FreeType and builds a glyph atlas texture.
- `Vertex` -- defines `Vertex`, `CircleVertex`, and `PickingVertex` structs and the `Color` struct.
- `BatchTypes` -- defines `SpriteGlyph`, `Batch`, and related types used by the sprite batcher.
- `Primitives` -- simple geometry helpers.
- `PickingTexture` -- integer framebuffer for mouse-over entity ID readback.
- `Framebuffer` -- general-purpose FBO used for offscreen render passes and effect ping-pong.

### Utils
- `OpenGLDebugger` -- sets up `glDebugMessageCallback` for driver-level error and warning output.

## Dependencies
`OpenGL 4.6`, `glad`, `glm`, `SDL3`, `FreeType`, `SCION_LOGGER`
