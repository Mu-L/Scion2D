#pragma once

namespace Scion::Editor
{

struct RecentProject
{
	std::string sName{};
	std::string sProjectPath{};	  // full path to the .s2dprj file
	std::string sLastOpenedStr{}; // formatted for display, e.g. "2025-06-14 09:30"
	std::int64_t lastOpenedUnix{ 0 };
	bool bPathExists{ false }; // validated at load-time, not persisted
};

// ============================================================
//  RecentProjects
// ============================================================
//
//  Persists the list of recently opened projects to:
//    Windows : %APPDATA%\Scion2D\recent_projects.json
//    Linux   : ~/.config/Scion2D/recent_projects.json
// ============================================================

class RecentProjects
{
  public:
	static constexpr int MAX_RECENTS = 10;

	RecentProjects();

	// Load from disk. Safe to call multiple times.
	void Load();

	// Persist to disk. Called automatically by mutating methods.
	void Save() const;

	// Add or update an entry (bump its timestamp to now and move to front).
	// projectFilePath should be the full path to the .s2dprj file.
	void AddOrUpdate( const std::string& projectFilePath );

	// Remove by index.
	void Remove( int index );

	// Remove all entries.
	void Clear();

	const std::vector<RecentProject>& GetProjects() const { return m_Projects; }
	std::vector<RecentProject>& GetProjects() { return m_Projects; }

	// Returns the platform-appropriate config directory, creating it if needed.
	static fs::path GetConfigDir();

  private:
	void ValidatePaths();
	static std::string FormatTimestamp( std::int64_t unixTs );
	static std::int64_t NowUnix();

	std::vector<RecentProject> m_Projects{};
	fs::path m_JsonPath{};
};

} // namespace Scion::Editor
