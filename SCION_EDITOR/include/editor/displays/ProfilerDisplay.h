#pragma once
#include "IDisplay.h"
#include "Core/Profiling/ProfileCollector.h"
#include <imgui.h>

namespace Scion::Editor
{
class ProfilerDisplay : public IDisplay
{
  public:
	ProfilerDisplay();
	~ProfilerDisplay() override;

	virtual void Draw() override;
	virtual void Update() override;

  protected:
	virtual void DrawToolbar() override;

  private:
	// -- Sub-panels
	void DrawFrameGraph();
	void DrawStatsTable( const std::vector<Scion::Core::ZoneStat>& stats );

	// -- Helpers
	static ImVec4 FrameTimeColor( float ms );
	static ImVec4 ZoneDepthColor( int depth );

  private:
	bool m_bPaused{ false };
	int m_FrameWindow{ 60 };		 // How many frames to average stats over
	int m_SelectedFrame{ -1 };		 // Frame index clicked in the graph
	float m_TargetFrameMs{ 16.67f }; // Budget line (default 60 FPS)

	// Cache stats so we don't recompute every frame
	std::vector<Scion::Core::ZoneStat> m_CachedStats{};
	int m_LastStatsFrame{ -1 };
};

} // namespace Scion::Editor
