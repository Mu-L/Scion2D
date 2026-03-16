#include "Sounds/Essentials/Audio.hpp"
#include "Logger/Logger.h"

namespace Scion::Sounds
{
Audio::Audio( MIX_Audio* pAudio, AudioType eType, const std::string& sFilename )
	: m_pAudio{ MakeUniqueFromSDLType<MIX_Audio>( pAudio ) }
	, m_sFilename{ sFilename }
	, m_eType{ eType }
{
	// Set All properties
	SDL_PropertiesID props = MIX_GetAudioProperties( pAudio );
	if ( props == 0 )
	{
		SCION_WARN( "Failed to set the properties for the audio: {}", SDL_GetError() );
		return;
	}

	m_Properties.sTitle =
		std::string{ SDL_GetStringProperty( props, MIX_PROP_METADATA_TITLE_STRING, "Unknown Title" ) };

	m_Properties.sArtistName =
		std::string{ SDL_GetStringProperty( props, MIX_PROP_METADATA_ARTIST_STRING, "Unknown Artist" ) };

	m_Properties.sAlbumName =
		std::string{ SDL_GetStringProperty( props, MIX_PROP_METADATA_ALBUM_STRING, "Unknown Album" ) };

	m_Properties.sCopyright =
		std::string{ SDL_GetStringProperty( props, MIX_PROP_METADATA_COPYRIGHT_STRING, "No Copyright" ) };

	m_Properties.trackNumber = SDL_GetNumberProperty( props, MIX_PROP_METADATA_TRACK_NUMBER, 0 );
	m_Properties.totalTracks = SDL_GetNumberProperty( props, MIX_PROP_METADATA_TOTAL_TRACKS_NUMBER, 0 );
	m_Properties.releasedDate = SDL_GetNumberProperty( props, MIX_PROP_METADATA_YEAR_NUMBER, 0 );
	m_Properties.durationFrames = SDL_GetNumberProperty( props, MIX_PROP_METADATA_DURATION_FRAMES_NUMBER, 0 );
}

Audio ::~Audio() = default;

} // namespace Scion::Sounds
