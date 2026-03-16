#include "ScionUtilities/SDL_Wrappers.h"
#include <iostream>
#include <Logger/Logger.h>

namespace Scion::Utilities
{

void SDL_Destroyer::operator()( SDL_Window* window ) const
{
	SDL_DestroyWindow( window );
	SCION_LOG( "Destroyed SDL WINDOW" );
}

void SDL_Destroyer::operator()( SDL_Gamepad* controller ) const
{
	SDL_CloseGamepad( controller );
	controller = nullptr;
	SCION_LOG( "Closed SDL Game Controller!" );
}

void SDL_Destroyer::operator()( MIX_Audio* audio ) const
{
	MIX_DestroyAudio( audio );
	SCION_LOG( "Destroyed SDL MIX_Audio!" );
}

void SDL_Destroyer::operator()( MIX_Mixer* pMixer ) const
{
	if ( pMixer )
	{
		SDL_PropertiesID props = MIX_GetMixerProperties( pMixer );
		SDL_AudioDeviceID devID = (SDL_AudioDeviceID)SDL_GetNumberProperty( props, MIX_PROP_MIXER_DEVICE_NUMBER, 0 );

		MIX_DestroyMixer( pMixer );
		SDL_CloseAudioDevice( devID );
		SCION_LOG( "Destroyed SDL Mixer" );
	}
}

void SDL_Destroyer::operator()( SDL_Cursor* cursor ) const
{
	if ( cursor )
	{
		SDL_DestroyCursor( cursor );
		SCION_LOG( "Destroyed SDL Cursor" );
	}
}

void SDL_Destroyer::operator()( SDL_AudioStream* stream ) const
{
	if ( stream )
	{
		SDL_DestroyAudioStream( stream );
		SCION_LOG( "Destroyed SDL Audio Stream" );
	}
}
void SDL_Destroyer::operator()( MIX_Track* track ) const
{
	if ( track )
	{
		MIX_DestroyTrack( track );
		SCION_LOG( "Destroyed SDL Mix Track" );
	}
}

} // namespace Scion::Utilities
