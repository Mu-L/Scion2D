include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# =====================================================
# SDL3_mixer
# =====================================================
if (MSVC)
	set(SDL3_DIR "${VCPKG_ROOT}/installed/x64-windows-static/lib/")
else()
	set(SDL3_DIR "${VCPKG_ROOT}/installed/x64-linux-static/lib/")
endif()

find_package(SDL3 CONFIG REQUIRED)

# Prefer static linking (avoids DLL issues)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Disable vendored libraries; use vcpkg packages
set(SDLMIXER_VENDORED OFF CACHE BOOL "" FORCE)

# MP3
set(SDLMIXER_MP3 ON CACHE BOOL "" FORCE)
set(SDLMIXER_MP3_MPG123 ON CACHE BOOL "" FORCE)  # Use system/vcpkg mpg123
set(SDLMIXER_MP3_DRMP3 ON CACHE BOOL "" FORCE)

# OGG / Vorbis
set(SDLMIXER_VORBIS_VORBISFILE ON CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_TREMOR OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_STB OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_VORBISFILE_SHARED OFF CACHE BOOL "" FORCE)

# FLAC (optional, for ogg/flac support)
set(SDLMIXER_FLAC_LIBFLAC ON CACHE BOOL "" FORCE)
set(SDLMIXER_FLAC_LIBFLAC_SHARED OFF CACHE BOOL "" FORCE)


FetchContent_Declare(
    SDL3_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG prerelease-3.1.2
)

FetchContent_MakeAvailable(SDL3_mixer)
