#pragma once
#include "Profiler.h"

#define PROFILE_COLLECTOR() Scion::Core::ProfileCollector::GetInstance()

namespace Scion::Core
{
/** @brief A single timing sample for one named zone, captured per frame */
struct ProfileSample
{
	std::string name{};
	float durationMs{ 0.f };
	int depth{ 0 }; // Nesting depth - 0 == Top Level
};

/** @brief Aggregated stats for a named zone, computed over the ring buffer window */
struct ZoneStat
{
	std::string name{};
	float avgMs{ 0.f };
	float minMs{ 0.f };
	float maxMs{ 0.f };
	float lastMs{ 0.f };
	int depth{ 0 };
};

/** @brief One frame's complete profile snapshot */
struct FrameProfile
{
	float totalMs{ 0.f };
	std::vector<ProfileSample> samples{};
};

/** @brief Ring buffer size - how many frames of history to retain. */
static constexpr int PROFILE_HISTORY_SIZE = 128;

/*
 * ProfileCollector
 * @brief Collects manual timing data from engine subsystems each frame.
 */
class ProfileCollector
{
  public:
	static ProfileCollector& GetInstance()
	{
		static ProfileCollector instance{};
		return instance;
	}

	ProfileCollector( const ProfileCollector& ) = delete;
	ProfileCollector& operator=( const ProfileCollector& ) = delete;

	/** @brief Call at the start of each engine frame (before any systems run). */
	void BeginFrame();

	/** @brief Call at the end of each engine frame (after all systems run). */
	void EndFrame();

	/** @brief Record the start of a named zone. Returns a token index. */
	int BeginZone( const std::string& name, int depth = 0 );

	/** @brief Record the end of the zone identified by the token. */
	void EndZone( int token );

	/** @brief Read-only access to the history ring buffer. */
	const std::array<FrameProfile, PROFILE_HISTORY_SIZE>& GetHistory() const { return m_History; }

	/** @brief Index of the most recently-completed frame in the ring buffer. */
	int GetCurrentIndex() const { return m_CurrentIndex; }

	/** @brief Frames captured so far (capped and PROFILE_HISTORY_SIZE). */
	int GetFrameCount() const { return m_FrameCount; }

	/** @brief Compute aggregated stats from the last N frames. */
	std::vector<ZoneStat> ComputeStats( int frameWindow = 60 ) const;

	/** @brief Current FPS (rolling average). */
	float GetFPS() const { return m_Fps; }

  private:
	ProfileCollector() = default;

	struct PendingZone
	{
		std::string name{};
		int depth{ 0 };
		std::chrono::high_resolution_clock::time_point start{};
	};

	std::array<FrameProfile, PROFILE_HISTORY_SIZE> m_History{};

	std::vector<PendingZone> m_Pending{};

	FrameProfile m_CurrentFrame{};
	std::chrono::high_resolution_clock::time_point m_FrameStart{};

	int m_CurrentIndex{ 0 };
	int m_FrameCount{ 0 };
	float m_Fps{ 0.f };
	float m_FpsAccum{ 0.f };
	int m_FpsSampleCount{ 0 };
};

/*
 * ScopedProfileZone
 * @brief RAII zone collector. Mirrors ZoneScoped but feeds the in-editor display
 */
struct ScopedProfileZone
{
	int token{ -1 };
	explicit ScopedProfileZone( const std::string& name, int depth = 0 )
		: token{ PROFILE_COLLECTOR().BeginZone( name, depth ) }
	{
	}

	~ScopedProfileZone() { PROFILE_COLLECTOR().EndZone( token ); }
};

} // namespace Scion::Core

// clang-format off


/*
* @brief Profiles a scope with both Tracy (for the external viewer) and
* the in-editor ProfileCollector (for our imgui display panel).
*/
#define SCION_PROFILE_SCOPE_EX( name, depth )						\
	SCION_PROFILE_SCOPE_N( name );									\
	Scion::Core::ScopedProfileZone _sz_##__LINE__{ name, depth }

/*
* @brief Top-level system zone (depth 0)
*/
#define SCION_SYSTEM_ZONE(name)  SCION_PROFILE_SCOPE_EX( name, 0)

/*
* @brief Sub-system zone (depth 1)
*/
#define SCION_SUBSYSTEM_ZONE(name)  SCION_PROFILE_SCOPE_EX( name, 1)
// clang-format on
