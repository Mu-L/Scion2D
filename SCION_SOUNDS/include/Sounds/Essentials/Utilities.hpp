#pragma once
#include <SDL3/SDL.h>

namespace Scion::Sounds
{
// -----------------------------------------------------------------------------
// DecodedAudio - Holds the raw PCM data decoded along with its format
// description. This struct owns the audio buffer via std::vector.
// -----------------------------------------------------------------------------
struct DecodedAudio
{
	// Raw interleaved PCM samples
	std::vector<uint8_t> pcmData;
	// Samples per second (e.g., 44100, 48000)
	uint32_t sampleRate = 0;
	// Number of audio channels (1=mono, 2=stereo, etc.)
	uint16_t channels = 0;
	// Bits per sample (typically 16 for PCM)
	uint16_t bitsPerSample = 0;
	// Bytes per complete sample frame (channels * bits/8)
	uint32_t blockAlign = 0;

	// Convenience: get a read-only span over the PCM data
	[[nodiscard]] std::span<const uint8_t> dataSpan() const { return std::span<const uint8_t>( pcmData ); }

	// Convenience: total duration in seconds
	[[nodiscard]] double durationSeconds() const
	{
		if ( sampleRate == 0 || blockAlign == 0 )
			return 0.0;

		const size_t totalFrames = pcmData.size() / blockAlign;
		return static_cast<double>( totalFrames ) / static_cast<double>( sampleRate );
	}
};

SDL_AudioSpec GetAudioSpecFromDecodedAudio( const DecodedAudio& audio );

} // namespace Scion::Sounds
