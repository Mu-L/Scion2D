#include "Core/Profiling/ProfileCollector.h"
#include <numeric>

namespace Scion::Core
{

void ProfileCollector::BeginFrame()
{
	m_FrameStart = std::chrono::high_resolution_clock::now();
	m_CurrentFrame = FrameProfile{};
	m_Pending.clear();
}

void ProfileCollector::EndFrame()
{
	auto now = std::chrono::high_resolution_clock::now();
	float frameMs = std::chrono::duration<float, std::milli>( now - m_FrameStart ).count();

	// Commit frame into the ring buffer
	m_CurrentIndex = ( m_CurrentIndex + 1 ) % PROFILE_HISTORY_SIZE;
	m_History[ m_CurrentIndex ] = std::move( m_CurrentFrame );
	m_FrameCount = std::min( m_FrameCount + 1, PROFILE_HISTORY_SIZE );

	// Rolling FPS
	m_FpsAccum += frameMs;
	++m_FpsSampleCount;
	if ( m_FpsSampleCount >= 10 )
	{
		m_Fps = 1000.f / ( m_FpsAccum / static_cast<float>( m_FpsSampleCount ) );
		m_FpsAccum = 0.f;
		m_FpsSampleCount = 0;
	}
}

int ProfileCollector::BeginZone( const std::string& name, int depth )
{
	int token = static_cast<int>( m_Pending.size() );
	m_Pending.push_back( { name, depth, std::chrono::high_resolution_clock::now() } );
	return token;
}

void ProfileCollector::EndZone( int token )
{
	if ( token < 0 || token >= static_cast<int>( m_Pending.size() ) )
	{
		return;
	}

	auto& pz = m_Pending[ token ];
	auto now = std::chrono::high_resolution_clock::now();
	float ms = std::chrono::duration<float, std::milli>( now - pz.start ).count();

	m_CurrentFrame.samples.push_back( { pz.name.empty() ? "?" : pz.name, ms, pz.depth } );
}

std::vector<ZoneStat> ProfileCollector::ComputeStats( int frameWindow ) const
{
	std::unordered_map<std::string, std::vector<float>> buckets;
	std::unordered_map<std::string, int> depthMap;

	int count = std::min( frameWindow, m_FrameCount );
	int start = m_CurrentIndex;

	for ( int i = 0; i < count; ++i )
	{
		int idx = ( start - i + PROFILE_HISTORY_SIZE ) % PROFILE_HISTORY_SIZE;
		for ( const auto& frameProf : m_History[ idx ].samples )
		{
			buckets[ frameProf.name ].push_back( frameProf.durationMs );
			depthMap[ frameProf.name ] = frameProf.depth;
		}
	}

	std::vector<ZoneStat> stats;
	stats.reserve( buckets.size() );

	for ( auto& [ name, times ] : buckets )
	{
		ZoneStat zStat;
		zStat.name = name;
		zStat.depth = depthMap[ name ];
		zStat.avgMs = std::accumulate( times.begin(), times.end(), 0.f ) / static_cast<float>( times.size() );
		zStat.minMs = *std::min_element( times.begin(), times.end() );
		zStat.maxMs = *std::max_element( times.begin(), times.end() );
		zStat.lastMs = times.front();

		stats.push_back( std::move( zStat ) );
	}

	// Sort: depth first, then avg cost descending
	std::ranges::sort( stats, []( const ZoneStat& a, const ZoneStat& b ) {
		if ( a.depth != b.depth )
			return a.depth < b.depth;

		return a.avgMs > b.avgMs;
	} );

	return stats;
}

} // namespace Scion::Core
