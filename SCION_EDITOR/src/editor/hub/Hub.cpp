#include "editor/hub/Hub.h"

#include <Windowing/Window/Window.h>
#include "editor/utilities/imgui/Gui.h"
#include "editor/utilities/imgui/ImGuiUtils.h"
#include "editor/utilities/EditorUtilities.h"
#include "editor/loaders/ProjectLoader.h"
#include "ScionFilesystem/Dialogs/FileDialog.h"
#include "Logger/Logger.h"

#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"

#include "ScionUtilities/HelperUtilities.h"

#include <Rendering/Essentials/Texture.h>

// IMGUI
// ===================================
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <SDL3/SDL_opengl.h>
// ===================================

// ============================================================
//  Layout & style constants
// ============================================================

static constexpr float BANNER_HEIGHT = 100.f;
static constexpr float STATUS_BAR_HEIGHT = 38.f;
static constexpr float LEFT_PANEL_RATIO = 0.55f; // fraction of total width
static constexpr float DIVIDER_W = 1.f;
static constexpr float CARD_H = 68.f;
static constexpr float BTN_H = 28.f;
static constexpr float BTN_W_WIDE = 150.f;
static constexpr float BTN_W_NARROW = 80.f;
static constexpr float FIELD_LABEL_W = 90.f; // left-align labels in form

// Colours
static const ImVec4 COL_BG{ 0.13f, 0.14f, 0.15f, 1.f };
static const ImVec4 COL_PANEL{ 0.10f, 0.11f, 0.12f, 1.f };
static const ImVec4 COL_BANNER{ 0.08f, 0.09f, 0.10f, 1.f };
static const ImVec4 COL_STATUSBAR{ 0.09f, 0.09f, 0.10f, 1.f };
static const ImVec4 COL_ACCENT{ 0.22f, 0.53f, 0.90f, 1.f };
static const ImVec4 COL_ACCENT_HOV{ 0.30f, 0.62f, 1.00f, 1.f };
static const ImVec4 COL_ACCENT_ACT{ 0.18f, 0.44f, 0.78f, 1.f };
static const ImVec4 COL_DIM{ 0.50f, 0.52f, 0.54f, 1.f };
static const ImVec4 COL_WARNING{ 0.90f, 0.60f, 0.20f, 1.f };
static const ImVec4 COL_ERROR{ 0.95f, 0.35f, 0.35f, 1.f };
static const ImVec4 COL_CARD_HOV{ 0.20f, 0.22f, 0.25f, 1.f };
static const ImVec4 COL_CARD_SEL{ 0.18f, 0.36f, 0.58f, 1.f };
static const ImVec4 COL_DANGER{ 0.65f, 0.15f, 0.15f, 1.f };
static const ImVec4 COL_DANGER_HOV{ 0.80f, 0.22f, 0.22f, 1.f };
static const ImVec4 COL_SEPARATOR{ 0.24f, 0.25f, 0.27f, 1.f };
static const ImVec4 COL_INPUT{ 0.16f, 0.17f, 0.19f, 1.f };

namespace Scion::Editor
{

Hub::Hub( Scion::Windowing::Window& window )
	: m_Window{ window }
	, m_bRunning{ false }
	, m_bLoadError{ false }
	, m_Event{}
	, m_eState{ EHubState::Default }
	, m_Width{ static_cast<float>( m_Window.GetWidth() ) }
	, m_Height{ static_cast<float>( m_Window.GetHeight() ) }
	, m_sNewProjectName{}
	, m_sNewProjectPath{ DEFAULT_PROJECT_PATH }
	, m_sPrevProjectPath{}
	, m_sPrevProjectName{}
{
	fs::path projectPath{ m_sNewProjectPath };
	if ( !fs::exists( projectPath ) )
	{
		std::error_code ec;
		if ( !fs::create_directories( projectPath, ec ) )
		{
			SCION_ERROR( "HUB - Failed to create directories. Error: {}", ec.message() );
			m_eState = EHubState::Close;
		}
	}
}

Hub::~Hub() = default;

bool Hub::Run()
{
	m_bRunning = Initialize();

	while ( m_bRunning )
	{
		ProcessEvents();
		Update();
		Render();
	}

	const bool bClosed{ m_eState == EHubState::Close };

	if ( !bClosed )
	{
		SDL_SetWindowBordered( m_Window.GetWindow().get(), true );
		SDL_SetWindowResizable( m_Window.GetWindow().get(), true );
		SDL_MaximizeWindow( m_Window.GetWindow().get() );

		std::string sTitle{ "Scion2D - " };
		sTitle += !m_sNewProjectName.empty() ? m_sNewProjectName : m_sPrevProjectName;
		SDL_SetWindowTitle( m_Window.GetWindow().get(), sTitle.c_str() );
	}

	return !bClosed;
}

bool Hub::Initialize()
{
	m_RecentProjects.Load();
	return true;
}

void Hub::ProcessEvents()
{
	while ( SDL_PollEvent( &m_Event ) )
	{
		ImGui_ImplSDL3_ProcessEvent( &m_Event );

		switch ( m_Event.type )
		{
		case SDL_EVENT_QUIT: m_bRunning = false; break;
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		default: break;
		}
	}
}

void Hub::Update()
{
	if ( m_PendingRemove >= 0 )
	{
		m_RecentProjects.Remove( m_PendingRemove );
		if ( m_SelectedRecent == m_PendingRemove )
			m_SelectedRecent = -1;
		else if ( m_SelectedRecent > m_PendingRemove )
			--m_SelectedRecent;

		m_PendingRemove = -1;
	}
}

void Hub::Render()
{
	Gui::Begin();
	DrawGui();
	Gui::End( &m_Window );

	SDL_GL_SwapWindow( m_Window.GetWindow().get() );
}

void Hub::DrawGui()
{
	// Push hub-wide style before opening the root window
	ImGui::PushStyleColor( ImGuiCol_WindowBg, COL_BG );
	ImGui::PushStyleColor( ImGuiCol_ChildBg, COL_PANEL );
	ImGui::PushStyleColor( ImGuiCol_Button, COL_ACCENT );
	ImGui::PushStyleColor( ImGuiCol_ButtonHovered, COL_ACCENT_HOV );
	ImGui::PushStyleColor( ImGuiCol_ButtonActive, COL_ACCENT_ACT );
	ImGui::PushStyleColor( ImGuiCol_FrameBg, COL_INPUT );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4{ 0.21f, 0.22f, 0.25f, 1.f } );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4{ 0.24f, 0.26f, 0.29f, 1.f } );
	ImGui::PushStyleColor( ImGuiCol_Separator, COL_SEPARATOR );
	ImGui::PushStyleColor( ImGuiCol_Header, COL_CARD_SEL );
	ImGui::PushStyleColor( ImGuiCol_HeaderHovered, COL_CARD_HOV );
	ImGui::PushStyleColor( ImGuiCol_HeaderActive, COL_CARD_SEL );
	ImGui::PushStyleColor( ImGuiCol_ScrollbarBg, COL_PANEL );
	ImGui::PushStyleColor( ImGuiCol_ScrollbarGrab, COL_ACCENT );
	ImGui::PushStyleColor( ImGuiCol_ScrollbarGrabHovered, COL_ACCENT_HOV );
	constexpr int NUM_COLORS = 15;

	ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.f );
	ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 4.f );
	ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 4.f );
	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2{ 0.f, 0.f } );
	ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2{ 8.f, 6.f } );
	constexpr int NUM_VARS = 5;

	constexpr ImGuiWindowFlags FLAGS = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
									   ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar |
									   ImGuiWindowFlags_NoScrollWithMouse;

	if ( !ImGui::Begin( "##HubRoot", nullptr, FLAGS ) )
	{
		ImGui::End();
		ImGui::PopStyleVar( NUM_VARS );
		ImGui::PopStyleColor( NUM_COLORS );
		return;
	}

	ImGui::SetWindowPos( ImGui::GetMainViewport()->Pos );
	ImGui::SetWindowSize( ImVec2{ m_Width, m_Height } );

	DrawBanner();

	const float contentH = m_Height - BANNER_HEIGHT - STATUS_BAR_HEIGHT;
	const float leftW = m_Width * LEFT_PANEL_RATIO;
	const float rightW = m_Width - leftW - DIVIDER_W;

	// Left panel
	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2{ 16.f, 14.f } );
	ImGui::BeginChild( "##LeftPanel", ImVec2{ leftW, contentH }, false );
	ImGui::PopStyleVar();
	DrawLeftPanel();
	ImGui::EndChild();

	// Divider
	ImGui::SameLine( 0.f, 0.f );
	ImGui::PushStyleColor( ImGuiCol_ChildBg, COL_SEPARATOR );
	ImGui::BeginChild( "##Divider", ImVec2{ DIVIDER_W, contentH }, false );
	ImGui::EndChild();
	ImGui::PopStyleColor();

	// Right panel
	ImGui::SameLine( 0.f, 0.f );
	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2{ 18.f, 14.f } );
	ImGui::BeginChild( "##RightPanel", ImVec2{ rightW, contentH }, false );
	ImGui::PopStyleVar();
	DrawRightPanel();
	ImGui::EndChild();

	DrawStatusBar();

	ImGui::End();
	ImGui::PopStyleVar( NUM_VARS );
	ImGui::PopStyleColor( NUM_COLORS );
}

void Hub::DrawBanner()
{
	ImGui::PushStyleColor( ImGuiCol_ChildBg, COL_BANNER );
	ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2{ 16.f, 0.f } );
	ImGui::BeginChild( "##Banner", ImVec2{ m_Width, BANNER_HEIGHT }, false );
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	// Logo texture
	auto pLogo = ASSET_MANAGER().GetTexture( "S2D_scion_logo" );
	if ( pLogo )
	{
		const float logoH = static_cast<float>( pLogo->GetHeight() ) * 1.5f;
		const float logoW = static_cast<float>( pLogo->GetWidth() ) * 1.5f;
		const float cursorY = ( BANNER_HEIGHT - logoH ) * 0.5f;
		ImGui::SetCursorPos( ImVec2{ 16.f, cursorY } );
		ImGui::Image( (ImTextureID)(intptr_t)pLogo->GetID(), ImVec2{ logoW, logoH } );
		ImGui::SameLine( 0.f, 14.f );
		ImGui::SetCursorPosY( ( BANNER_HEIGHT - ImGui::GetTextLineHeight() ) * 0.5f );
	}
	else
	{
		// Fallback text if logo isn't ready yet
		ImGui::SetCursorPos( ImVec2{ 16.f, ( BANNER_HEIGHT - ImGui::GetTextLineHeight() ) * 0.5f } );
		ImGui::PushStyleColor( ImGuiCol_Text, COL_ACCENT );
		ImGui::SetWindowFontScale( 1.5f );
		ImGui::Text( "SCION 2D" );
		ImGui::SetWindowFontScale( 1.f );
		ImGui::PopStyleColor();
		ImGui::SameLine( 0.f, 10.f );
	}

	ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
	ImGui::Text( "Project Hub" );
	ImGui::PopStyleColor();

	ImGui::EndChild();

	// 2px accent underline
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 wPos = ImGui::GetWindowPos();
	dl->AddRectFilled( { wPos.x, wPos.y + BANNER_HEIGHT },
					   { wPos.x + m_Width, wPos.y + BANNER_HEIGHT + 2.f },
					   ImGui::ColorConvertFloat4ToU32( COL_ACCENT ) );
}

void Hub::DrawLeftPanel()
{
	auto& projects = m_RecentProjects.GetProjects();
	const float panelW = ImGui::GetContentRegionAvail().x;

	// Section header
	ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
	ImGui::Text( "RECENT PROJECTS" );
	ImGui::PopStyleColor();

	if ( !projects.empty() )
	{
		ImGui::SameLine( panelW - 72.f );
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0, 0, 0, 0 } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, COL_CARD_HOV );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, COL_CARD_SEL );
		ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
		if ( ImGui::SmallButton( "Clear All" ) )
		{
			m_RecentProjects.Clear();
			m_SelectedRecent = -1;
		}
		ImGui::PopStyleColor( 4 );
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	DrawRecentProjectsList();

	// ---- Action buttons at bottom of left panel ----
	ImGui::Separator();
	ImGui::Spacing();

	const float halfW = ( panelW - 8.f ) * 0.5f;
	const auto& projs = m_RecentProjects.GetProjects();
	const bool bCanOpen = ( m_SelectedRecent >= 0 && m_SelectedRecent < static_cast<int>( projs.size() ) &&
							projs[ m_SelectedRecent ].bPathExists );

	// Open Selected
	if ( !bCanOpen )
	{
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.22f, 0.22f, 0.22f, 1.f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.22f, 0.22f, 0.22f, 1.f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0.22f, 0.22f, 0.22f, 1.f } );
		ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
	}

	if ( ImGui::Button( "Open Selected", ImVec2{ halfW, BTN_H } ) && bCanOpen )
		OpenRecentProject( m_SelectedRecent );

	if ( !bCanOpen )
		ImGui::PopStyleColor( 4 );

	ImGui::SameLine( 0.f, 8.f );

	// Browse for any project file
	if ( ImGui::Button( "Browse...", ImVec2{ halfW, BTN_H } ) )
	{
		if ( BrowseForProjectFile() )
		{
			// AddOrUpdate the browsed path then open it
			m_RecentProjects.AddOrUpdate( m_sPrevProjectPath );

			ProjectLoader pl{};
			if ( !pl.LoadProject( m_sPrevProjectPath ) )
			{
				m_bShowLoadError = true;
				m_sErrorMsg = "Failed to load project: " + m_sPrevProjectPath;
			}
			else
			{
				m_sPrevProjectName = fs::path{ m_sPrevProjectPath }.stem().string();
				m_bRunning = false;
				m_eState = EHubState::OpenProject;
			}
		}
	}

	if ( m_bShowLoadError )
	{
		ImGui::Spacing();
		ImGui::PushStyleColor( ImGuiCol_Text, COL_ERROR );
		ImGui::TextWrapped( "%s", m_sErrorMsg.c_str() );
		ImGui::PopStyleColor();
	}
}

void Hub::DrawRecentProjectsList()
{
	const auto& projects = m_RecentProjects.GetProjects();
	const float panelW = ImGui::GetContentRegionAvail().x;

	// Reserve space: total height minus the bottom buttons area (~BTN_H + spacing)
	const float listH = ImGui::GetContentRegionAvail().y - BTN_H - 32.f;

	if ( projects.empty() )
	{
		ImGui::BeginChild( "##EmptyList", ImVec2{ 0.f, listH }, false );
		ImGui::SetCursorPosY( listH * 0.35f );
		ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
		const char* msg = "No recent projects";
		ImGui::SetCursorPosX( ( panelW - ImGui::CalcTextSize( msg ).x ) * 0.5f );
		ImGui::TextUnformatted( msg );
		ImGui::PopStyleColor();
		ImGui::EndChild();
		return;
	}

	ImGui::BeginChild( "##ProjectList", ImVec2{ 0.f, listH }, false );

	for ( int i = 0; i < static_cast<int>( projects.size() ); ++i )
	{
		const auto& proj = projects[ i ];
		const bool bSelected = ( m_SelectedRecent == i );

		ImGui::PushID( i );

		// Card background rect
		ImVec2 cardMin = ImGui::GetCursorScreenPos();
		ImVec2 cardMax = { cardMin.x + ImGui::GetContentRegionAvail().x, cardMin.y + CARD_H };

		ImDrawList* dl = ImGui::GetWindowDrawList();
		const bool bHov = ImGui::IsMouseHoveringRect( cardMin, cardMax );

		ImU32 bgCol = bSelected ? ImGui::ColorConvertFloat4ToU32( COL_CARD_SEL )
								: ( bHov ? ImGui::ColorConvertFloat4ToU32( COL_CARD_HOV ) : IM_COL32( 0, 0, 0, 0 ) );

		dl->AddRectFilled( cardMin, cardMax, bgCol, 4.f );

		// Card contents: name + path + date
		constexpr float PAD_X = 10.f;
		constexpr float DELETE_AREA = 52.f;
		const float textW = cardMax.x - cardMin.x - PAD_X * 2.f - DELETE_AREA;

		ImGui::SetCursorPosX( ImGui::GetCursorPosX() + PAD_X );
		ImGui::SetCursorPosY( ImGui::GetCursorPosY() + 6.f );

		ImGui::BeginGroup();

		// Project name – amber if path is missing
		if ( !proj.bPathExists )
			ImGui::PushStyleColor( ImGuiCol_Text, COL_WARNING );

		ImGui::TextUnformatted( proj.sName.empty() ? "(Unnamed)" : proj.sName.c_str() );

		if ( !proj.bPathExists )
			ImGui::PopStyleColor();

		// Path (dimmed, truncated)
		ImGui::PushStyleColor( ImGuiCol_Text, proj.bPathExists ? COL_DIM : COL_WARNING );
		std::string displayPath = proj.sProjectPath;
		while ( !displayPath.empty() && ImGui::CalcTextSize( displayPath.c_str() ).x > textW )
		{
			// Trim from the left keeping the end visible
			displayPath = "..." + displayPath.substr( std::min<size_t>( displayPath.size(), 5 ) );
			if ( displayPath.size() <= 5 )
				break;
		}
		ImGui::TextUnformatted( displayPath.c_str() );
		ImGui::PopStyleColor();

		ImGui::EndGroup();

		// Invisible selectable over the card (leave room for delete btn)
		ImGui::SetCursorScreenPos( cardMin );
		if ( ImGui::InvisibleButton( "##cardsel", ImVec2{ cardMax.x - cardMin.x - DELETE_AREA, CARD_H } ) )
		{
			m_SelectedRecent = i;
			m_bShowLoadError = false;
			m_bShowCreateError = false;
		}

		// Double-click opens immediately
		if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( 0 ) )
			OpenRecentProject( i );

		// Date label (top-right of card)
		const std::string shortDate =
			proj.sLastOpenedStr.size() >= 10 ? proj.sLastOpenedStr.substr( 0, 10 ) : proj.sLastOpenedStr;

		//  Date label - draw directly, don't move cursor
		dl->AddText( { cardMax.x - DELETE_AREA + 2.f, cardMin.y + 6.f },
					 ImGui::ColorConvertFloat4ToU32( COL_DIM ),
					 shortDate.c_str() );

		// Delete button (bottom-right of card)
		ImGui::SetCursorScreenPos( { cardMax.x - DELETE_AREA + 8.f, cardMin.y + 30.f } );
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0, 0, 0, 0 } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, COL_DANGER );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, COL_DANGER_HOV );
		ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
		if ( ImGui::SmallButton( "x" ) )
			m_PendingRemove = i;
		if ( ImGui::IsItemHovered() )
			ImGui::SetTooltip( "Remove from recent list" );
		ImGui::PopStyleColor( 4 );

		// Move cursor past card
		ImGui::SetCursorScreenPos( { cardMin.x, cardMax.y + 2.f } );
		ImGui::Dummy( ImVec2{ ImGui::GetContentRegionAvail().x, 0.f } );

		// Tooltip for missing paths
		if ( !proj.bPathExists && bHov )
		{
			ImGui::BeginTooltip();
			ImGui::PushStyleColor( ImGuiCol_Text, COL_WARNING );
			ImGui::Text( "Project not found:" );
			ImGui::PopStyleColor();
			ImGui::TextUnformatted( proj.sProjectPath.c_str() );
			ImGui::EndTooltip();
		}

		ImGui::PopID();
		ImGui::Dummy( ImVec2{ 0.f, 2.f } );
	}

	ImGui::EndChild();
}

void Hub::DrawRightPanel()
{
	const float panelW = ImGui::GetContentRegionAvail().x;

	// Tab-style selector: two buttons at the top acting as tab headers
	const bool bNewActive = ( m_eState != EHubState::OpenProject );
	const bool bOpenActive = ( m_eState == EHubState::OpenProject );

	auto PushTabStyle = [ & ]( bool active ) {
		if ( active )
		{
			ImGui::PushStyleColor( ImGuiCol_Button, COL_ACCENT );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, COL_ACCENT_HOV );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, COL_ACCENT_ACT );
		}
		else
		{
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.17f, 0.18f, 0.20f, 1.f } );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, COL_CARD_HOV );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, COL_CARD_SEL );
		}
	};

	const float tabW = ( panelW - 4.f ) * 0.5f;
	PushTabStyle( bNewActive );
	if ( ImGui::Button( "New Project", ImVec2{ tabW, BTN_H } ) )
		m_eState = EHubState::NewProject;
	ImGui::PopStyleColor( 3 );

	ImGui::SameLine( 0.f, 4.f );

	PushTabStyle( bOpenActive );
	if ( ImGui::Button( "Open Project", ImVec2{ tabW, BTN_H } ) )
		m_eState = EHubState::OpenProject;
	ImGui::PopStyleColor( 3 );

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if ( m_eState == EHubState::OpenProject )
		DrawOpenProject();
	else
		DrawNewProject();
}

void Hub::DrawNewProject()
{
	const float panelW = ImGui::GetContentRegionAvail().x;
	const float inputW = panelW - FIELD_LABEL_W - 8.f;

	ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
	ImGui::Text( "Name" );
	ImGui::PopStyleColor();
	ImGui::SameLine( FIELD_LABEL_W );
	ImGui::SetNextItemWidth( inputW );

	char nameBuf[ 256 ]{ 0 };
#ifdef _WIN32
	strcpy_s( nameBuf, m_sNewProjectName.c_str() );
#else
	strcpy( nameBuf, m_sNewProjectName.c_str() );
#endif
	if ( ImGui::InputText( "##NewName", nameBuf, sizeof( nameBuf ) ) )
		m_sNewProjectName = std::string{ nameBuf };

	ImGui::Spacing();

	ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
	ImGui::Text( "Location" );
	ImGui::PopStyleColor();
	ImGui::SameLine( FIELD_LABEL_W );
	ImGui::SetNextItemWidth( inputW - BTN_W_NARROW - 6.f );

	char pathBuf[ 512 ]{ 0 };
#ifdef _WIN32
	strcpy_s( pathBuf, m_sNewProjectPath.c_str() );
#else
	strcpy( pathBuf, m_sNewProjectPath.c_str() );
#endif
	if ( ImGui::InputText( "##NewPath", pathBuf, sizeof( pathBuf ) ) )
		m_sNewProjectPath = std::string{ pathBuf };

	ImGui::SameLine( 0.f, 6.f );
	if ( ImGui::Button( "Browse##new", ImVec2{ BTN_W_NARROW, BTN_H } ) )
	{
		std::string chosen;
		if ( BrowseForFolder( chosen ) )
			m_sNewProjectPath = chosen;
	}

	ImGui::Spacing();
	if ( !m_sNewProjectName.empty() && !m_sNewProjectPath.empty() )
	{
		const fs::path fullPath = fs::path{ m_sNewProjectPath } / m_sNewProjectName;
		ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
		ImGui::TextWrapped( "Path: %s", fullPath.string().c_str() );
		ImGui::PopStyleColor();
	}

	ImGui::Spacing();

	if ( !m_sNewProjectName.empty() && !m_sNewProjectPath.empty() )
	{
		const fs::path projectPath{ m_sNewProjectPath };

		if ( fs::exists( projectPath ) )
		{
			if ( IsReservedPathOrFile( projectPath ) )
			{
				ImGui::PushStyleColor( ImGuiCol_Text, COL_ERROR );
				ImGui::TextWrapped( "Path [%s] is reserved. Please choose a different location.",
									m_sNewProjectPath.c_str() );
				ImGui::PopStyleColor();
			}
			else
			{
				if ( ImGui::Button( "Create Project", ImVec2{ panelW, BTN_H } ) )
				{
					ProjectLoader pl{};
					if ( !pl.CreateNewProject( m_sNewProjectName, m_sNewProjectPath ) )
					{
						m_bShowCreateError = true;
						m_sErrorMsg = "Failed to create project.";
					}
					else
					{
						// Record in recents
						const fs::path projFile = fs::path{ m_sNewProjectPath } / m_sNewProjectName / "SCION_2D" /
												  ( m_sNewProjectName + ".s2dprj" );
						m_RecentProjects.AddOrUpdate( projFile.string() );

						m_bRunning = false;
						m_eState = EHubState::CreateNew;
					}
				}
			}
		}
		else
		{
			ImGui::PushStyleColor( ImGuiCol_Text, COL_WARNING );
			ImGui::TextWrapped( "Location does not exist. It will be created on project creation." );
			ImGui::PopStyleColor();
			ImGui::Spacing();

			if ( ImGui::Button( "Create Project", ImVec2{ panelW, BTN_H } ) )
			{
				ProjectLoader pl{};
				if ( !pl.CreateNewProject( m_sNewProjectName, m_sNewProjectPath ) )
				{
					m_bShowCreateError = true;
					m_sErrorMsg = "Failed to create project.";
				}
				else
				{
					const fs::path projFile =
						fs::path{ m_sNewProjectPath } / m_sNewProjectName / ( m_sNewProjectName + ".s2dprj" );
					m_RecentProjects.AddOrUpdate( projFile.string() );

					m_bRunning = false;
					m_eState = EHubState::CreateNew;
				}
			}
		}
	}

	if ( m_bShowCreateError )
	{
		ImGui::Spacing();
		ImGui::PushStyleColor( ImGuiCol_Text, COL_ERROR );
		ImGui::TextWrapped( "%s", m_sErrorMsg.c_str() );
		ImGui::PopStyleColor();
	}
}

void Hub::DrawOpenProject()
{
	const float panelW = ImGui::GetContentRegionAvail().x;

	ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
	ImGui::TextWrapped( "Browse for an existing Scion2D project file (.s2dprj)." );
	ImGui::PopStyleColor();
	ImGui::Spacing();

	// Show whatever path is currently selected
	char prevBuf[ 512 ]{ 0 };
#ifdef _WIN32
	strcpy_s( prevBuf, m_sPrevProjectPath.c_str() );
#else
	strcpy( prevBuf, m_sPrevProjectPath.c_str() );
#endif
	ImGui::SetNextItemWidth( panelW - BTN_W_NARROW - 6.f );
	if ( ImGui::InputText( "##PrevPath", prevBuf, sizeof( prevBuf ) ) )
	{
		m_sPrevProjectPath = std::string{ prevBuf };
		m_sPrevProjectName = fs::path{ m_sPrevProjectPath }.stem().string();
	}

	ImGui::SameLine( 0.f, 6.f );
	if ( ImGui::Button( "Browse##open", ImVec2{ BTN_W_NARROW, BTN_H } ) )
	{
		BrowseForProjectFile();
	}

	ImGui::Spacing();

	// Load button – only active when a valid path is set
	if ( !m_sPrevProjectPath.empty() && fs::exists( fs::path{ m_sPrevProjectPath } ) )
	{
		if ( ImGui::Button( "Load Project", ImVec2{ panelW, BTN_H } ) )
		{
			ProjectLoader pl{};
			if ( !pl.LoadProject( m_sPrevProjectPath ) )
			{
				m_bShowLoadError = true;
				m_sErrorMsg = "Failed to load project.";
			}
			else
			{
				m_RecentProjects.AddOrUpdate( m_sPrevProjectPath );
				m_bRunning = false;
				m_eState = EHubState::OpenProject;
			}
		}
	}
	else if ( !m_sPrevProjectPath.empty() )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, COL_WARNING );
		ImGui::TextWrapped( "File not found: %s", m_sPrevProjectPath.c_str() );
		ImGui::PopStyleColor();
	}

	if ( m_bShowLoadError )
	{
		ImGui::Spacing();
		ImGui::PushStyleColor( ImGuiCol_Text, COL_ERROR );
		ImGui::TextWrapped( "%s", m_sErrorMsg.c_str() );
		ImGui::PopStyleColor();
	}
}

void Hub::DrawStatusBar()
{
	ImGui::PushStyleColor( ImGuiCol_ChildBg, COL_STATUSBAR );
	ImGui::BeginChild( "##StatusBar", ImVec2{ m_Width, STATUS_BAR_HEIGHT }, false );
	ImGui::PopStyleColor();

	ImGui::SetCursorPosY( ( STATUS_BAR_HEIGHT - ImGui::GetTextLineHeight() ) * 0.5f );
	ImGui::SetCursorPosX( 12.f );
	ImGui::PushStyleColor( ImGuiCol_Text, COL_DIM );
	ImGui::Text( "Scion2D" );
	ImGui::PopStyleColor();

	// Close button – right-aligned
	const float btnX = m_Width - BTN_W_NARROW - 8.f;
	ImGui::SetCursorPos( ImVec2{ btnX, ( STATUS_BAR_HEIGHT - BTN_H ) * 0.5f } );
	ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0, 0, 0, 0 } );
	ImGui::PushStyleColor( ImGuiCol_ButtonHovered, COL_DANGER );
	ImGui::PushStyleColor( ImGuiCol_ButtonActive, COL_DANGER_HOV );

	if ( ImGui::Button( "Close", ImVec2{ BTN_W_NARROW, BTN_H } ) )
	{
		m_eState = EHubState::Close;
		m_bRunning = false;
	}

	ImGui::PopStyleColor( 3 );

	ImGui::EndChild();
}

void Hub::OpenRecentProject( int index )
{
	const auto& projects = m_RecentProjects.GetProjects();
	if ( index < 0 || index >= static_cast<int>( projects.size() ) )
		return;

	const auto& proj = projects[ index ];
	if ( !proj.bPathExists )
		return;

	ProjectLoader pl{};
	if ( !pl.LoadProject( proj.sProjectPath ) )
	{
		m_bShowLoadError = true;
		m_sErrorMsg = "Failed to load: " + proj.sProjectPath;
		return;
	}

	m_RecentProjects.AddOrUpdate( proj.sProjectPath );
	m_sPrevProjectPath = proj.sProjectPath;
	m_sPrevProjectName = proj.sName;
	m_bRunning = false;
	m_eState = EHubState::OpenProject;
}

bool Hub::BrowseForProjectFile()
{
	Scion::Filesystem::FileDialog fd{};
	const auto sFilepath = fd.OpenFileDialog( "Open Project", "", { "*.s2dprj" }, "Scion2D Proj (*.s2dprj" );
	if ( sFilepath.empty() )
	{
		return false;
	}

	m_sPrevProjectPath = sFilepath;
	m_sPrevProjectName = fs::path{ sFilepath }.stem().string();
	m_bShowLoadError = false;
	return true;
}

bool Hub::BrowseForFolder( std::string& outPath )
{
	Scion::Filesystem::FileDialog fd{};

	const auto sPath = fd.SelectFolderDialog( "Select Location", BASE_PATH );
	if ( sPath.empty() )
	{
		return false;
	}

	outPath = sPath;
	return true;
}

} // namespace Scion::Editor
