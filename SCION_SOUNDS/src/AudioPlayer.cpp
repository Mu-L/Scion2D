#include "Sounds/AudioPlayer/AudioPlayer.hpp"
#include "Logger/Logger.h"

namespace Scion::Sounds
{

AudioPlayer::AudioPlayer( int numTracks, const SDL_AudioSpec& spec )
	: m_NumTracks{ numTracks }
{
	SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice( SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec );
	if ( audioDevice == 0 )
	{
		SCION_ERROR( "Failed to open audio: {}", SDL_GetError() );
	}
	else
	{
		// Start audio playback
		SDL_PauseAudioDevice( audioDevice );
	}

	m_pMixer = MakeUniqueFromSDLType<MIX_Mixer>( MIX_CreateMixerDevice( audioDevice, NULL ) );

	if ( !m_pMixer )
	{
		throw std::runtime_error( "Failed to create audio player. Mixer was invalid" );
	}

	if ( m_NumTracks > 128 )
	{
		SCION_WARN( "Only 128 tracks are supported. 128 Tracks created" );
		m_NumTracks = 128;
	}

	if ( m_NumTracks < 0 )
	{
		SCION_ERROR( "Number of tracks must be greater than zero. No Tracks created" );
		m_NumTracks = 0;
	}

	// Create the default channels
	for ( int i = 0; i < m_NumTracks; ++i )
	{
		m_Tracks[ i ] = MIX_CreateTrack( m_pMixer.get() );
	}
}

AudioPlayer::~AudioPlayer()
{
	for ( auto* pTrack : m_Tracks )
	{
		if ( pTrack )
		{
			MIX_DestroyTrack( pTrack );
		}
	}
}

bool AudioPlayer::PlayTrack( int trackNum, int loops )
{
	if ( trackNum < 0 || trackNum >= m_NumTracks || trackNum >= m_Tracks.size() )
	{
		SCION_ERROR( "Trying to play an invalid track" );
		return false;
	}

	auto* pTrack = m_Tracks[ trackNum ];
	if ( !pTrack )
	{
		SCION_ERROR( "Track audio for track {} has not been set", trackNum );
		return false;
	}

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty( props, MIX_PROP_PLAY_LOOPS_NUMBER, loops );
	if ( !MIX_PlayTrack( pTrack, props ) )
	{
		SCION_ERROR( "Failed to play track: {}", SDL_GetError() );
		SDL_DestroyProperties( props );
		return false;
	}

	SDL_DestroyProperties( props );
	return true;
}

bool AudioPlayer::PlayTrack( int trackNum, MIX_Audio* pAudio, int loops )
{
	if ( trackNum < -1 || trackNum >= m_NumTracks || trackNum >= static_cast<int>( m_Tracks.size() ) )
	{
		SCION_ERROR( "Trying to play an invalid track" );
		return false;
	}

	if ( !pAudio )
	{
		SCION_ERROR( "Audio passed in for for track {} is invalid", trackNum );
		return false;
	}

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty( props, MIX_PROP_PLAY_LOOPS_NUMBER, loops );

	// If tracknum == -1 find the first non-playing track
	if ( trackNum == -1 )
	{
		bool bPlayedTrack{ false };
		for ( size_t i = 0; i < m_NumTracks; ++i )
		{
			if ( !MIX_TrackPlaying( m_Tracks[ i ] ) )
			{
				if ( !MIX_SetTrackAudio( m_Tracks[ i ], pAudio ) )
				{
					SCION_ERROR( "Failed to set audio for track {}. ", i );
					return false;
				}

				if ( !MIX_PlayTrack( m_Tracks[ i ], props ) )
				{
					SCION_ERROR( "Failed to play track: {}", SDL_GetError() );
					SDL_DestroyProperties( props );
					return false;
				}

				bPlayedTrack = true;
				break;
			}
		}

		if ( !bPlayedTrack )
		{
			SCION_ERROR( "Failed to play track. All tracks are in use." );
			SDL_DestroyProperties( props );
			return false;
		}
	}
	else
	{
		if ( !MIX_SetTrackAudio( m_Tracks[ trackNum ], pAudio ) )
		{
			SCION_ERROR( "Failed to set audio for track {}. ", trackNum );
			return false;
		}

		if ( !MIX_PlayTrack( m_Tracks[ trackNum ], props ) )
		{
			SCION_ERROR( "Failed to play track: {}", SDL_GetError() );
			SDL_DestroyProperties( props );
			return false;
		}
	}

	SDL_DestroyProperties( props );
	return true;
}

bool AudioPlayer::FadeInAudio( int trackNum, MIX_Audio* pAudio, int milliseconds )
{
	if ( trackNum < 0 || trackNum >= m_NumTracks || trackNum >= m_Tracks.size() )
	{
		SCION_WARN( "Trying to play an invalid track" );
		return false;
	}

	if ( !pAudio )
	{
		SCION_ERROR( "Audio passed in for for track {} is invalid", trackNum );
		return false;
	}

	if ( !MIX_SetTrackAudio( m_Tracks[ trackNum ], pAudio ) )
	{
		SCION_ERROR( "Failed to set audio for track {}. ", trackNum );
		return false;
	}

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty( props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, milliseconds );

	if ( !MIX_PlayTrack( m_Tracks[ trackNum ], props ) )
	{
		SCION_ERROR( "Failed to fade in track track: {}", SDL_GetError() );
		SDL_DestroyProperties( props );
		return false;
	}

	SDL_DestroyProperties( props );
	return true;
}

bool AudioPlayer::StopTrack( int trackNum, int fadeOutFrames )
{
	if ( trackNum < 0 || trackNum >= m_NumTracks || trackNum >= m_Tracks.size() || !m_Tracks[ trackNum ] )
	{
		SCION_ERROR( "Failed to stop track [{}] - Track is invalid.", trackNum );
		return false;
	}

	return MIX_StopTrack( m_Tracks[ trackNum ], fadeOutFrames );
}

void AudioPlayer::StopAllTracks()
{
	for ( const auto& track : m_Tracks )
	{
		if ( MIX_TrackPlaying( track ) && !MIX_StopTrack( track, 0 ) )
		{
			SCION_WARN( "Failed to stop track." );
		}
	}
}

bool AudioPlayer::IsTrackPlaying( int trackNum )
{
	if ( trackNum < 0 || trackNum >= m_NumTracks || trackNum >= m_Tracks.size() || !m_Tracks[ trackNum ] )
	{
		SCION_ERROR( "Failed to check if track is playing. Track [{}] is invalid.", trackNum );
		return false;
	}

	return MIX_TrackPlaying( m_Tracks[ trackNum ] );
}

bool AudioPlayer::IsTrackPaused( int trackNum )
{
	if ( trackNum < 0 || trackNum >= m_NumTracks || trackNum >= m_Tracks.size() || !m_Tracks[ trackNum ] )
	{
		SCION_ERROR( "Failed to check if track is paused. Track [{}] is invalid.", trackNum );
		return false;
	}

	return MIX_TrackPaused( m_Tracks[ trackNum ] );
}

bool AudioPlayer::PauseTrack( int trackNum )
{
	if ( trackNum < 0 || trackNum >= m_NumTracks || trackNum >= m_Tracks.size() || !m_Tracks[ trackNum ] )
	{
		SCION_ERROR( "Failed to pause track. Track [{}] is invalid.", trackNum );
		return false;
	}

	return MIX_PauseTrack( m_Tracks[ trackNum ] );
}

bool AudioPlayer::ResumeTrack( int trackNum )
{
	if ( trackNum < 0 || trackNum >= m_NumTracks || trackNum >= m_Tracks.size() || !m_Tracks[ trackNum ] )
	{
		SCION_ERROR( "Failed to resume track. Track [{}] is invalid.", trackNum );
		return false;
	}

	return MIX_ResumeTrack( m_Tracks[ trackNum ] );
}

bool AudioPlayer::SetTrackVolume( int trackNum, float volume )
{
	if ( trackNum < 0 || trackNum >= m_NumTracks || trackNum >= m_Tracks.size() || !m_Tracks[ trackNum ] )
	{
		SCION_ERROR( "Failed to set track volume. Track [{}] is invalid.", trackNum );
		return false;
	}

	if ( volume < 0.f )
	{
		volume = 0.f;
	}

	return MIX_SetTrackGain( m_Tracks[ trackNum ], volume );
}

bool AudioPlayer::SetMaxTracksCount( int numTracks )
{
	if (numTracks < 0 || numTracks > static_cast<int>(m_Tracks.size()))
	{
		SCION_ERROR( "Failed to set max tracks. Num Tracks [{}] is invalid.", numTracks );
		return false;
	}

	if ( numTracks == m_NumTracks )
	{
		SCION_WARN( "Failed to set max tracks to [{}]. Max tracks is already set to that value.", numTracks );
		return false;
	}

	if (numTracks < m_NumTracks)
	{
		for (int i = m_NumTracks - 1; i >= numTracks; --i)
		{
			if (auto* pTrack = m_Tracks[i])
			{
				MIX_DestroyTrack( pTrack );	
			}
		}
	}
	else if (numTracks > m_NumTracks)
	{
		for ( int i = m_NumTracks; i < numTracks; ++i )
		{
			// TODO: Add error handling encase this fails
			m_Tracks[ i ]= MIX_CreateTrack( m_pMixer.get() );
		}
	}

	m_NumTracks = numTracks;
	return true;
}

bool AudioPlayer::SetTrackAudio( int trackNum, MIX_Audio* pAudio )
{
	if ( trackNum < 0 || trackNum >= m_NumTracks || trackNum >= static_cast<int>( m_Tracks.size() ) )
	{
		SCION_ERROR( "Trying to set track audio for an invalid track" );
		return false;
	}

	if ( !pAudio )
	{
		SCION_ERROR( "Audio passed in for for track {} is invalid", trackNum );
		return false;
	}

	if ( !MIX_SetTrackAudio( m_Tracks[ trackNum ], pAudio ) )
	{
		SCION_ERROR( "Failed to set audio for track {}. ", trackNum );
		return false;
	}

	return true;
}

bool AudioPlayer::SetMasterVolume( float gain )
{
	gain = std::clamp( gain, 0.f, 0.5f );
	return MIX_SetMixerGain( m_pMixer.get(), gain );
}

} // namespace Scion::Sounds
