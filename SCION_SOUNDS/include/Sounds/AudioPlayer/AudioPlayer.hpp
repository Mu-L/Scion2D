#pragma once
#include <ScionUtilities/SDL_Wrappers.h>
#include <sol/sol.hpp>

namespace Scion::Sounds
{
class AudioPlayer
{
  public:
	AudioPlayer( int numTracks, const SDL_AudioSpec& spec );
	~AudioPlayer();

	bool PlayTrack( int trackNum, int loops = 0 );
	bool PlayTrack( int trackNum, MIX_Audio* pAudio, int loops = 0 );

	bool FadeInAudio( int trackNum, MIX_Audio* pAudio, int milliseconds );
	bool StopTrack( int trackNum, int fadeOutFrames = 0 );
	void StopAllTracks();

	bool IsTrackPlaying( int trackNum );
	bool IsTrackPaused( int trackNum );
	bool PauseTrack( int trackNum );
	bool ResumeTrack( int trackNum );

	bool SetTrackVolume( int trackNum, float volume );

	bool SetMasterVolume( float gain );

	inline bool PauseAll() { return MIX_PauseAllTracks( m_pMixer.get() ); }
	inline bool ResumeAll() { return MIX_ResumeAllTracks( m_pMixer.get() ); }
	inline MIX_Mixer* GetMixer() { return m_pMixer.get(); }

  private:
	MixerUPtr m_pMixer{ nullptr };

	int m_NumTracks{ 16 };
	std::array<MIX_Track*, 128> m_Tracks{ nullptr };
};

} // namespace Scion::Sounds
