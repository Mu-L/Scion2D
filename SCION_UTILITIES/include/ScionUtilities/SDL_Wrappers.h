#pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

namespace Scion::Utilities
{
/*
 * @brief Simple wrapper for SDL C-style types.
 * @brief These are custom deleters that allow us to use shared pointers to
 * handle the clean up of the SDL types
 */
struct SDL_Destroyer
{
	void operator()( SDL_Window* window ) const;
	void operator()( SDL_Gamepad* controller ) const;
	void operator()( SDL_Cursor* cursor ) const;
	void operator()( MIX_Audio* audio ) const;
	void operator()( MIX_Mixer* audio ) const;
	void operator()( SDL_AudioStream* stream ) const;
	void operator()( MIX_Track* track ) const;
};
} // namespace Scion::Utilities

// Useful Aliases
using GamepadUPtr = std::unique_ptr<SDL_Gamepad, Scion::Utilities::SDL_Destroyer>;
using CursorUPtr = std::unique_ptr<SDL_Cursor, Scion::Utilities::SDL_Destroyer>;
using WindowUPtr = std::unique_ptr<SDL_Window, Scion::Utilities::SDL_Destroyer>;
using AudioUPtr = std::unique_ptr<MIX_Audio, Scion::Utilities::SDL_Destroyer>;
using MixerUPtr = std::unique_ptr<MIX_Mixer, Scion::Utilities::SDL_Destroyer>;
using TrackUPtr = std::unique_ptr<MIX_Track, Scion::Utilities::SDL_Destroyer>;
using AudioStreamUPtr = std::unique_ptr<SDL_AudioStream, Scion::Utilities::SDL_Destroyer>;

/**
 * @brief Creates a std::shared_ptr from a raw SDL pointer with a custom deleter.
 *
 * This utility function wraps a raw SDL pointer in a std::shared_ptr using the
 * Scion::Utilities::SDL_Destroyer as the deleter. This ensures the SDL resource is
 * properly released when the shared pointer goes out of scope.
 *
 * @tparam RPtr      The return pointer type, typically deduced as std::shared_ptr<TSDLType>.
 * @tparam TSDLType  The type of the raw SDL resource.
 * @param pSDLType   The raw pointer to the SDL resource to manage.
 * @return A std::shared_ptr managing the SDL resource with a custom deleter.
 */
template <typename RPtr, typename TSDLType>
inline RPtr MakeSharedFromSDLType( TSDLType* pSDLType )
{
	return std::shared_ptr<TSDLType>( pSDLType, Scion::Utilities::SDL_Destroyer{} );
}

template <typename TSdlType, auto DeleterFn>
using SdlPtr = std::unique_ptr<TSdlType, decltype( DeleterFn )>;

template <typename TSdlType, auto DeleterFn = Scion::Utilities::SDL_Destroyer{}>
SdlPtr<TSdlType, DeleterFn> MakeUniqueFromSDLType( TSdlType* ptr )
{
	return SdlPtr<TSdlType, DeleterFn>( ptr, DeleterFn );
}
