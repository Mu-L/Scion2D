#pragma once
#include <ScionUtilities/SDL_Wrappers.h>

namespace Scion::Sounds
{

struct AudioProperties
{
	std::string sTitle{};
	std::string sArtistName{};
	std::string sAlbumName{};
	std::string sCopyright{};

	int trackNumber{ 0 };
	int totalTracks{ 0 };
	int releasedDate{ 0 };
	int durationFrames{ 0 };
};

enum class AudioType
{
	None,
	Music,
	Soundfx
};

class Audio
{
  public:
	Audio( MIX_Audio* pAudio, AudioType eType, const std::string& sFilename );
	~Audio();

	inline const std::string& GetTitle() const { return m_Properties.sTitle; }
	inline const std::string& GetArtistName() const { return m_Properties.sArtistName; }
	inline const std::string& GetAlbumName() const { return m_Properties.sAlbumName; }
	inline const std::string& GetCopyright() const { return m_Properties.sCopyright; }
	inline const int GetTrackNumber() const { return m_Properties.trackNumber; }
	inline const int GetTotalTracks() const { return m_Properties.totalTracks; }
	inline const int GetReleasedDate() const { return m_Properties.releasedDate; }
	inline const int GetDurationFrames() const { return m_Properties.durationFrames; }
	inline const std::string& GetFilename() const { return m_sFilename; }
	inline const AudioType GetType() const { return m_eType; }

	[[nodiscard]] inline MIX_Audio* GetAudioPtr() const
	{
		if ( !m_pAudio )
			return nullptr;

		return m_pAudio.get();
	}

  private:
	AudioUPtr m_pAudio;
	AudioProperties m_Properties;
	std::string m_sFilename;
	AudioType m_eType;
};
} // namespace Scion::Sounds
