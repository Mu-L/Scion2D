#include "editor/displays/ProfilerDisplay.h"
#include "editor/utilities/EditorState.h"
#include "editor/utilities/fonts/IconsFontAwesome5.h"

#include "Core/ECS/MainRegistry.h"

using namespace Scion::Core;

namespace Scion::Editor
{
ProfilerDisplay::ProfilerDisplay()
{
}

ProfilerDisplay::~ProfilerDisplay()
{
}

void ProfilerDisplay::Draw()
{
	if ( auto& pEditorState = MAIN_REGISTRY().GetContext<EditorStatePtr>() )
	{
		if ( !pEditorState->IsDisplayOpen( EDisplay::ProfilerDisplay ) )
			return;
	}

	if ( !ImGui::Begin( ICON_FA_TACHOMETER_ALT " Profiler###ProfilerDisplay" ) )
	{
		ImGui::End();
		return;
	}

	auto& collector = PROFILE_COLLECTOR();

	// Refresh cached stats once per frame (unless paused)
	int curIdx = collector.GetCurrentIndex();
	if ( !m_bPaused && curIdx != m_LastStatsFrame )
	{
		m_CachedStats = collector.ComputeStats( m_FrameWindow );
		m_LastStatsFrame = curIdx;
	}

	DrawToolbar();
	ImGui::Separator();

	// Determine which frame to show in flame/stats
	const FrameProfile& activeFrame =
		( m_SelectedFrame >= 0 ) ? collector.GetHistory()[ m_SelectedFrame ] : collector.GetHistory()[ curIdx ];

	if ( ImGui::BeginTabBar( "##profiler_tabs" ) )
	{
		if ( ImGui::BeginTabItem( "Zone Stats" ) )
		{
			DrawStatsTable( m_CachedStats );
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void ProfilerDisplay::Update()
{
}

void ProfilerDisplay::DrawToolbar()
{
	auto& collector = PROFILE_COLLECTOR();

	// FPS / frame time badge
	float fps = collector.GetFPS();
	int curIdx = collector.GetCurrentIndex();
	float lastMs = collector.GetHistory()[ curIdx ].totalMs;

	ImGui::TextColored( FrameTimeColor( lastMs ), "%.1f FPS  |  %.2f ms", fps, lastMs );

	ImGui::SameLine( 0.f, 20.f );

	// Pause toggle
	if ( m_bPaused )
	{
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.2f, 0.6f, 0.2f, 1.f ) );
		if ( ImGui::Button( "  Resume  " ) )
		{
			m_bPaused = false;
		}

		ImGui::PopStyleColor();
	}
	else
	{
		if ( ImGui::Button( "  Pause  " ) )
		{
			m_bPaused = true;
			m_SelectedFrame = curIdx;
		}
	}

	ImGui::SameLine();
	ImGui::SetNextItemWidth( 100.f );
	ImGui::SliderInt( "Window", &m_FrameWindow, 10, PROFILE_HISTORY_SIZE, "%d frames" );

	ImGui::SameLine();
	ImGui::SetNextItemWidth( 80.f );
	ImGui::InputFloat( "Budget ms", &m_TargetFrameMs, 0.f, 0.f, "%.2f" );
}

void ProfilerDisplay::DrawFrameGraph()
{
	auto& collector = PROFILE_COLLECTOR();
	int count = collector.GetFrameCount();
	int curIdx = collector.GetCurrentIndex();

	if ( count == 0 )
	{
		ImGui::TextDisabled( "No frame data yet." );
		return;
	}

	// Build float array for PlotHistogram (most-recent on the right)
	static float values[ PROFILE_HISTORY_SIZE ];
	for ( int i = 0; i < count; ++i )
	{
		int idx = ( curIdx - i + PROFILE_HISTORY_SIZE ) % PROFILE_HISTORY_SIZE;
		values[ count - 1 - i ] = collector.GetHistory()[ idx ].totalMs;
	}

	ImVec2 graphSize( ImGui::GetContentRegionAvail().x, 80.f );

	// Draw budget line as an overlay
	ImVec2 p0 = ImGui::GetCursorScreenPos();
	ImGui::PlotHistogram( "##framegraph", values, count, 0, nullptr, 0.f, m_TargetFrameMs * 2.f, graphSize );

	// Budget line overlay
	float barH = graphSize.y;
	float barW = graphSize.x;
	float lineY = p0.y + barH - ( m_TargetFrameMs / ( m_TargetFrameMs * 2.f ) ) * barH;
	ImGui::GetWindowDrawList()->AddLine(
		ImVec2( p0.x, lineY ), ImVec2( p0.x + barW, lineY ), IM_COL32( 255, 80, 80, 180 ), 1.5f );

	// Click to select a frame
	if ( ImGui::IsItemHovered() && ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
	{
		float mouseX = ImGui::GetIO().MousePos.x - p0.x;
		int clickedBucket = static_cast<int>( mouseX / barW * static_cast<float>( count ) );
		clickedBucket = std::clamp( clickedBucket, 0, count - 1 );
		// Map back from display order to ring-buffer index
		m_SelectedFrame = ( curIdx - ( count - 1 - clickedBucket ) + PROFILE_HISTORY_SIZE ) % PROFILE_HISTORY_SIZE;
		m_bPaused = true;
	}

	ImGui::TextDisabled( "Click a bar to freeze and inspect that frame. Red line = %.2f ms budget.", m_TargetFrameMs );
}

void ProfilerDisplay::DrawStatsTable( const std::vector<ZoneStat>& stats )
{
	if ( stats.empty() )
	{
		ImGui::TextDisabled( "No zones recorded. Add SCION_SYSTEM_ZONE() macros to your systems." );
		return;
	}

	constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
									  ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Sortable;

	float tableH = ImGui::GetContentRegionAvail().y - 30.f;

	if ( ImGui::BeginTable( "##zone_stats", 6, flags, ImVec2( 0.f, tableH ) ) )
	{
		ImGui::TableSetupScrollFreeze( 0, 1 );
		ImGui::TableSetupColumn( "Zone", ImGuiTableColumnFlags_WidthStretch );
		ImGui::TableSetupColumn( "Avg ms", ImGuiTableColumnFlags_WidthFixed, 70.f );
		ImGui::TableSetupColumn( "Min ms", ImGuiTableColumnFlags_WidthFixed, 70.f );
		ImGui::TableSetupColumn( "Max ms", ImGuiTableColumnFlags_WidthFixed, 70.f );
		ImGui::TableSetupColumn( "Last ms", ImGuiTableColumnFlags_WidthFixed, 70.f );
		ImGui::TableSetupColumn( "Bar", ImGuiTableColumnFlags_WidthFixed, 120.f );
		ImGui::TableHeadersRow();

		// Find max avg for relative bar scaling
		float maxAvg = 0.01f;
		for ( const auto& zone : stats )
			maxAvg = std::max( maxAvg, zone.avgMs );

		for ( const auto& zone : stats )
		{
			ImGui::TableNextRow();

			// Zone name — indent by depth
			ImGui::TableSetColumnIndex( 0 );
			ImGui::Indent( static_cast<float>( zone.depth ) * 14.f );
			ImGui::TextColored( ZoneDepthColor( zone.depth ), "%s", zone.name.c_str() );
			ImGui::Unindent( static_cast<float>( zone.depth ) * 14.f );

			ImGui::TableSetColumnIndex( 1 );
			ImGui::TextColored( FrameTimeColor( zone.avgMs ), "%.3f", zone.avgMs );

			ImGui::TableSetColumnIndex( 2 );
			ImGui::TextDisabled( "%.3f", zone.minMs );

			ImGui::TableSetColumnIndex( 3 );
			ImGui::TextColored( FrameTimeColor( zone.maxMs ), "%.3f", zone.maxMs );

			ImGui::TableSetColumnIndex( 4 );
			ImGui::Text( "%.3f", zone.lastMs );

			// Relative bar
			ImGui::TableSetColumnIndex( 5 );
			float frac = zone.avgMs / maxAvg;
			ImVec4 barCol = FrameTimeColor( zone.avgMs );
			ImGui::PushStyleColor( ImGuiCol_PlotHistogram, barCol );
			ImGui::ProgressBar( frac, ImVec2( -1.f, 0.f ), "" );
			ImGui::PopStyleColor();
		}

		ImGui::EndTable();
	}
}

ImVec4 ProfilerDisplay::FrameTimeColor( float ms )
{
	if ( ms < 8.f )
		return ImVec4( 0.3f, 0.9f, 0.3f, 1.f ); // green  < 8 ms
	if ( ms < 16.f )
		return ImVec4( 0.9f, 0.8f, 0.1f, 1.f ); // yellow 8–16 ms
	if ( ms < 33.f )
		return ImVec4( 1.0f, 0.5f, 0.1f, 1.f ); // orange 16–33 ms
	return ImVec4( 1.0f, 0.2f, 0.2f, 1.f );		// red    > 33 ms
}

ImVec4 ProfilerDisplay::ZoneDepthColor( int depth )
{
	static const ImVec4 palette[] = {
		{ 0.25f, 0.55f, 0.90f, 1.f }, // depth 0 — blue     (engine systems)
		{ 0.20f, 0.75f, 0.60f, 1.f }, // depth 1 — teal     (sub-systems)
		{ 0.40f, 0.85f, 0.35f, 1.f }, // depth 2 — green    (Lua zones)
		{ 0.85f, 0.70f, 0.20f, 1.f }, // depth 3 — gold     (detail)
		{ 0.75f, 0.35f, 0.75f, 1.f }, // depth 4 — purple   (micro)
	};
	int idx = std::clamp( depth, 0, 4 );
	return palette[ idx ];
}

} // namespace Scion::Editor
