#include "Sounds/Essentials/Utilities.hpp"

namespace Scion::Sounds
{
SDL_AudioSpec GetAudioSpecFromDecodedAudio( const DecodedAudio& audio )
{
	SDL_AudioSpec spec{};

	// Map bits-per-sample to the appropriate SDL audio format
	switch ( audio.bitsPerSample )
	{
	case 8:
		// 8-bit PCM from WMF is unsigned
		spec.format = SDL_AUDIO_U8;
		break;
	case 16:
		// 16-bit PCM is the most common format; signed little-endian
		spec.format = SDL_AUDIO_S16;
		break;
	case 24:
		// SDL3 doesn't have a native 24-bit format.
		// In practice, WMF with MFAudioFormat_PCM and 24-bit is rare.
		// If you need 24-bit support, request MFAudioFormat_Float instead.
		// For now, we'll error out cleanly.
		throw std::runtime_error( "24-bit PCM is not directly supported by SDL3. "
								  "Consider requesting float output from WMF instead." );
	case 32:
		// 32-bit PCM integer
		spec.format = SDL_AUDIO_S32;
		break;
	default: throw std::runtime_error( "Unsupported bits-per-sample: " + std::to_string( audio.bitsPerSample ) );
	}

	// Channel count (SDL3 uses int for channels)
	spec.channels = static_cast<int>( audio.channels );

	// Sample rate
	spec.freq = static_cast<int>( audio.sampleRate );

	return spec;
}
} // namespace Scion::Sounds
