#include "editor/hub/RecentProjects.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <cstdlib>
#include <pwd.h>
#include <unistd.h>
#endif

namespace Scion::Editor
{

fs::path RecentProjects::GetConfigDir()
{
	fs::path configDir;

#ifdef _WIN32
	PWSTR pAppData = nullptr;
	if ( SUCCEEDED( SHGetKnownFolderPath( FOLDERID_RoamingAppData, 0, nullptr, &pAppData ) ) )
	{
		configDir = fs::path( pAppData ) / "Scion2D";
		CoTaskMemFree( pAppData );
	}
	else
	{
		const char* pFallback = std::getenv( "APPDATA" );
		configDir = pFallback ? fs::path( pFallback ) / "Scion2D" : fs::current_path() / ".scion2d";
	}
#else
	const char* pXdg = std::getenv( "XDG_CONFIG_HOME" );
	if ( pXdg && pXdg[ 0 ] != '\0' )
	{
		configDir = fs::path( pXdg ) / "Scion2D";
	}
	else
	{
		const char* pHome = std::getenv( "HOME" );
		if ( !pHome )
		{
			struct passwd* pw = getpwuid( getuid() );
			pHome = pw ? pw->pw_dir : ".";
		}
		configDir = fs::path( pHome ) / ".config" / "Scion2D";
	}
#endif

	std::error_code ec;
	fs::create_directories( configDir, ec );
	return configDir;
}

std::int64_t RecentProjects::NowUnix()
{
	using namespace std::chrono;
	return static_cast<std::int64_t>( duration_cast<seconds>( system_clock::now().time_since_epoch() ).count() );
}

std::string RecentProjects::FormatTimestamp( std::int64_t unixTs )
{
	std::time_t t = static_cast<std::time_t>( unixTs );
	std::tm tmBuf{};
#ifdef _WIN32
	localtime_s( &tmBuf, &t );
#else
	localtime_r( &t, &tmBuf );
#endif
	std::ostringstream oss;
	oss << std::put_time( &tmBuf, "%Y-%m-%d %H:%M" );
	return oss.str();
}

RecentProjects::RecentProjects()
{
	m_JsonPath = GetConfigDir() / "recent_projects.json";
}

void RecentProjects::Load()
{
	m_Projects.clear();

	if ( !fs::exists( m_JsonPath ) )
		return;

	std::ifstream ifs( m_JsonPath );
	if ( !ifs.is_open() )
		return;

	const std::string content{ std::istreambuf_iterator<char>( ifs ), std::istreambuf_iterator<char>() };

	rapidjson::Document doc;
	doc.Parse( content.c_str() );

	if ( doc.HasParseError() || !doc.IsObject() )
		return;

	if ( !doc.HasMember( "projects" ) || !doc[ "projects" ].IsArray() )
		return;

	const auto& arr = doc[ "projects" ];
	for ( rapidjson::SizeType i = 0; i < arr.Size(); ++i )
	{
		const auto& obj = arr[ i ];
		if ( !obj.IsObject() )
			continue;

		RecentProject proj;
		if ( obj.HasMember( "name" ) && obj[ "name" ].IsString() )
			proj.sName = obj[ "name" ].GetString();
		if ( obj.HasMember( "projectPath" ) && obj[ "projectPath" ].IsString() )
			proj.sProjectPath = obj[ "projectPath" ].GetString();
		if ( obj.HasMember( "lastOpenedUnix" ) && obj[ "lastOpenedUnix" ].IsInt64() )
			proj.lastOpenedUnix = obj[ "lastOpenedUnix" ].GetInt64();

		proj.sLastOpenedStr = FormatTimestamp( proj.lastOpenedUnix );
		m_Projects.push_back( std::move( proj ) );
	}

	std::sort( m_Projects.begin(), m_Projects.end(), []( const RecentProject& a, const RecentProject& b ) {
		return a.lastOpenedUnix > b.lastOpenedUnix;
	} );

	ValidatePaths();
}

void RecentProjects::Save() const
{
	rapidjson::Document doc;
	doc.SetObject();
	auto& alloc = doc.GetAllocator();

	rapidjson::Value arr( rapidjson::kArrayType );
	for ( const auto& proj : m_Projects )
	{
		rapidjson::Value obj( rapidjson::kObjectType );
		obj.AddMember( "name", rapidjson::Value( proj.sName.c_str(), alloc ), alloc );
		obj.AddMember( "projectPath", rapidjson::Value( proj.sProjectPath.c_str(), alloc ), alloc );
		obj.AddMember( "lastOpenedUnix", rapidjson::Value( proj.lastOpenedUnix ), alloc );
		arr.PushBack( obj, alloc );
	}
	doc.AddMember( "projects", arr, alloc );

	rapidjson::StringBuffer sb;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( sb );
	doc.Accept( writer );

	std::ofstream ofs( m_JsonPath );
	if ( ofs.is_open() )
	{
		ofs << sb.GetString();
	}
}

void RecentProjects::AddOrUpdate( const std::string& projectFilePath )
{
	for ( auto& proj : m_Projects )
	{
		if ( proj.sProjectPath == projectFilePath )
		{
			proj.lastOpenedUnix = NowUnix();
			proj.sLastOpenedStr = FormatTimestamp( proj.lastOpenedUnix );
			proj.bPathExists = fs::exists( proj.sProjectPath );

			std::sort( m_Projects.begin(), m_Projects.end(), []( const RecentProject& a, const RecentProject& b ) {
				return a.lastOpenedUnix > b.lastOpenedUnix;
			} );

			Save();
			return;
		}
	}

	RecentProject proj;
	proj.sProjectPath = projectFilePath;
	proj.lastOpenedUnix = NowUnix();
	proj.sLastOpenedStr = FormatTimestamp( proj.lastOpenedUnix );
	proj.bPathExists = fs::exists( projectFilePath );
	proj.sName = fs::path( projectFilePath ).stem().string();

	m_Projects.insert( m_Projects.begin(), std::move( proj ) );

	if ( static_cast<int>( m_Projects.size() ) > MAX_RECENTS )
		m_Projects.resize( MAX_RECENTS );

	Save();
}

void RecentProjects::Remove( int index )
{
	if ( index < 0 || index >= static_cast<int>( m_Projects.size() ) )
		return;
	m_Projects.erase( m_Projects.begin() + index );
	Save();
}

void RecentProjects::Clear()
{
	m_Projects.clear();
	Save();
}

void RecentProjects::ValidatePaths()
{
	for ( auto& proj : m_Projects )
	{
		proj.bPathExists = fs::exists( proj.sProjectPath );
	}
}

} // namespace Scion::Editor
