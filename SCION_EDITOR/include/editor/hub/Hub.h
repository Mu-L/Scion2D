#pragma once
#include <SDL3/SDL.h>
#include "editor/hub/RecentProjects.h"

namespace Scion::Windowing
{
class Window;
}

namespace Scion::Editor
{

enum class EHubState
{
	Default,
	NewProject,
	CreateNew,
	OpenProject,
	Close,
	NoState
};

class Hub
{
  public:
	Hub( Scion::Windowing::Window& window );
	~Hub();

	bool Run();

  private:
	bool Initialize();
	void DrawGui();
	void ProcessEvents();
	void Update();
	void Render();

	void DrawLeftPanel();
	void DrawRightPanel();
	void DrawNewProject();
	void DrawOpenProject();
	void DrawRecentProjectsList();
	void DrawBanner();
	void DrawStatusBar();

	void OpenRecentProject( int index );
	void TryLoadSelectedRecent();
	bool BrowseForProjectFile();
	bool BrowseForFolder( std::string& outPath );

  private:
	Scion::Windowing::Window& m_Window;
	bool m_bRunning;
	bool m_bLoadError;
	SDL_Event m_Event;
	EHubState m_eState;

	float m_Width;
	float m_Height;

	std::string m_sNewProjectName;
	std::string m_sNewProjectPath;
	std::string m_sPrevProjectPath;
	std::string m_sPrevProjectName;

	RecentProjects m_RecentProjects{};
	int m_SelectedRecent{ -1 };
	int m_PendingRemove{ -1 };
	bool m_bShowCreateError{ false };
	bool m_bShowLoadError{ false };

	std::string m_sErrorMsg{};
};

} // namespace Scion::Editor
