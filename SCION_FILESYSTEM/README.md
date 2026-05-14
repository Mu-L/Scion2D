# SCION_FILESYSTEM

Filesystem utilities for Scion2D projects. Handles file dialogs, JSON and Lua serialization,
directory watching, and platform-specific file operations.

## What It Contains

- `JSONSerializer` -- reads and writes `nlohmann::json` objects to/from files.
- `LuaSerializer` -- writes Lua table literals to files for data-driven content.
- `FileDialog` -- wraps a native OS file dialog (open/save). Windows-only for now.
- `FileProcessor` -- platform-specific file operations (copy, move, delete). Split into
  `FileProcessor_Win.cpp` and `FileProcessor_Unix.cpp`; the CMake build selects the correct one.
- `DirectoryWatcher` -- watches a directory for file changes and invokes a callback. Used by
  the editor's content browser to detect new or modified assets.
- `FilesystemUtilities` -- helper functions: path normalization, extension checks, directory creation.

## Dependencies
`std::filesystem`, `nlohmann/json`, `sol2`, `SCION_LOGGER`
