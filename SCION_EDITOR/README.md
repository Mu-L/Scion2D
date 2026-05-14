# SCION_EDITOR

The editor application. Built on top of SCION_CORE and rendered with ImGui docking layout.
This is the tool used to build scenes, manage assets, and script game objects for projects
targeting Scion2D.

## What It Contains

### Displays (ImGui panels)
Each display is a panel inside the docking workspace.
- `SceneDisplay` -- the main viewport showing the active scene. Handles camera pan/zoom.
- `SceneHierarchyDisplay` -- entity tree with folder support, drag-and-drop, and right-click context menus.
- `EntityDisplay` -- entity inspector. Shows and edits all attached components.
- `AssetDisplay` -- asset browser. Shows textures, fonts, shaders, audio, and prefabs.
- `ContentDisplay` -- project file browser for navigating the content folder.
- `TilemapDisplay` -- tilemap painting viewport with layer management.
- `AnimationDisplay` -- sprite animation editor.
- `LogDisplay` -- live log viewer.
- `MenuDisplay` -- top menu bar (File, Edit, View, Tools).
- `PackageDisplay` -- game packaging and export settings.
- `EditorStyleToolDisplay` -- ImGui theme editor.

### Commands (undo/redo)
- `CommandManager` -- manages an undo stack and redo stack. All editor mutations go through a command.
- Tile commands: add tile, remove tile, rect fill tile, create tile tool commands.

### Tools
- `TileTool` -- single-tile paint tool.
- `RectFillTool` -- rectangular fill tool.
- `CreateTileTool` -- creates a new tile entity from the palette.
- `ToolManager` -- owns and switches between active tools.
- Gizmos: `TranslateGizmo`, `RotateGizmo`, `ScaleGizmo` -- in-viewport transform handles.

### Systems
- `GridSystem` -- renders the tilemap grid overlay in the scene viewport.

### Utilities
- `DrawComponentUtils` -- ImGui helpers for rendering each component's property panel.
- `EditorUtilities` -- shared editor helper functions.
- `ImGuiUtils` -- wrappers for common ImGui patterns used across panels.
- `EditorState` -- holds global editor state (selected entity, active scene, active tool, etc.).

## Entry Point
`SCION_EDITOR/src/main.cpp` initializes SDL3, OpenGL, ImGui, and the main registry, then runs the editor loop.

## Dependencies
`SCION_CORE`, `SCION_RENDERING`, `SCION_FILESYSTEM`, `SCION_LOGGER`, `SCION_UTILITIES`, `ImGui` (docking), `SDL3`, `OpenGL 4.6`
