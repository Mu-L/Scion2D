#include "Core/Resources/AssetManager.h"
#include "Core/Resources/fonts/default_fonts.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/CoreUtilities/Prefab.h"

#include <Rendering/Essentials/TextureLoader.h>
#include <Rendering/Essentials/ShaderLoader.h>
#include <Rendering/Essentials/FontLoader.h>
#include <ScionUtilities/ScionUtilities.h>
#include <Logger/Logger.h>

namespace SCION_RESOURCES
{

bool AssetManager::CreateDefaultFonts()
{
	if ( !AddFontFromMemory( "pixel", pixel_font ) )
	{
		SCION_ERROR( "Failed to create pixel font." );
		return false;
	}

	// TODO: Add more default fonts.

	return true;
}

bool AssetManager::AddTexture( const std::string& textureName, const std::string& texturePath, bool pixelArt,
							   bool bTileset )
{
	// Check to see if the texture already exists
	if ( m_mapTextures.find( textureName ) != m_mapTextures.end() )
	{
		SCION_ERROR( "Failed to add texture [{0}] -- Already exists!", textureName );
		return false;
	}

	auto texture = SCION_RENDERING::TextureLoader::Create( pixelArt ? SCION_RENDERING::Texture::TextureType::PIXEL
																	: SCION_RENDERING::Texture::TextureType::BLENDED,
														   texturePath,
														   bTileset );

	if ( !texture )
	{
		SCION_ERROR( "Failed to load texture [{0}] at path [{1}]", textureName, texturePath );
		return false;
	}

	auto [ itr, bSuccess ] = m_mapTextures.emplace( textureName, std::move( texture ) );
	return bSuccess;
}

bool AssetManager::AddTextureFromMemory( const std::string& textureName, const unsigned char* imageData, size_t length,
										 bool pixelArt, bool bTileset )
{
	// Check to see if the Texture already exist
	if ( m_mapTextures.contains( textureName ) )
	{
		SCION_ERROR( "AssetManager: Texture [{}] -- Already exists!", textureName );
		return false;
	}

	auto texture =
		std::move( SCION_RENDERING::TextureLoader::CreateFromMemory( imageData, length, pixelArt, bTileset ) );

	// Load the texture
	if ( !texture )
	{
		SCION_ERROR( "Unable to load texture [{}] from memory!", textureName );
		return false;
	}

	// Insert the texture into the map
	auto [ itr, bSuccess ] = m_mapTextures.emplace( textureName, std::move( texture ) );

	return bSuccess;
}

std::shared_ptr<SCION_RENDERING::Texture> AssetManager::GetTexture( const std::string& textureName )
{
	auto texItr = m_mapTextures.find( textureName );
	if ( texItr == m_mapTextures.end() )
	{
		SCION_ERROR( "Failed to get texture [{0}] -- Does not exist!", textureName );
		return nullptr;
	}

	return texItr->second;
}

std::vector<std::string> AssetManager::GetTilesetNames() const
{
	return SCION_UTIL::GetKeys( m_mapTextures, []( const auto& pair ) { return pair.second->IsTileset(); } );
}

bool AssetManager::AddFont( const std::string& fontName, const std::string& fontPath, float fontSize )
{
	if ( m_mapFonts.contains( fontName ) )
	{
		SCION_ERROR( "Failed to add font [{0}] -- Already Exists!", fontName );
		return false;
	}

	auto pFont = SCION_RENDERING::FontLoader::Create( fontPath, fontSize );

	if ( !pFont )
	{
		SCION_ERROR( "Failed to add font [{}] at path [{}] -- to the asset manager!", fontName, fontPath );
		return false;
	}

	auto [ itr, bSuccess ] = m_mapFonts.emplace( fontName, std::move( pFont ) );

	return bSuccess;
}

bool AssetManager::AddFontFromMemory( const std::string& fontName, unsigned char* fontData, float fontSize )
{

	if ( m_mapFonts.contains( fontName ) )
	{
		SCION_ERROR( "Failed to add font [{0}] -- Already Exists!", fontName );
		return false;
	}

	auto pFont = SCION_RENDERING::FontLoader::CreateFromMemory( fontData, fontSize );

	if ( !pFont )
	{
		SCION_ERROR( "Failed to add font [{0}] from memory -- to the asset manager!", fontName );
		return false;
	}

	auto [ itr, bSuccess ] = m_mapFonts.emplace( fontName, std::move( pFont ) );

	return bSuccess;
}

std::shared_ptr<SCION_RENDERING::Font> AssetManager::GetFont( const std::string& fontName )
{
	auto fontItr = m_mapFonts.find( fontName );
	if ( fontItr == m_mapFonts.end() )
	{
		SCION_ERROR( "Failed to get font [{0}] -- Does not exist!", fontName );
		return nullptr;
	}

	return fontItr->second;
}

bool AssetManager::AddShader( const std::string& shaderName, const std::string& vertexPath,
							  const std::string& fragmentPath )
{
	// Check to see if the shader already exists
	if ( m_mapShader.contains( shaderName ) )
	{
		SCION_ERROR( "Failed to add shader [{0}] -- Already Exists!", shaderName );
		return false;
	}

	// Create and load the shader
	auto shader = std::move( SCION_RENDERING::ShaderLoader::Create( vertexPath, fragmentPath ) );

	if ( !shader )
	{
		SCION_ERROR( "Failed to load Shader [{0}] at vert path [{1}] and frag path [{2}]",
					 shaderName,
					 vertexPath,
					 fragmentPath );
		return false;
	}

	auto [ itr, bSuccess ] = m_mapShader.emplace( shaderName, std::move( shader ) );
	return bSuccess;
}

bool AssetManager::AddShaderFromMemory( const std::string& shaderName, const char* vertexShader,
										const char* fragShader )
{
	if ( m_mapShader.contains( shaderName ) )
	{
		SCION_ERROR( "Failed to add shader - [{0}] -- Already exists!", shaderName );
		return false;
	}

	auto shader = std::move( SCION_RENDERING::ShaderLoader::CreateFromMemory( vertexShader, fragShader ) );
	auto [ itr, bSuccess ] = m_mapShader.insert( std::make_pair( shaderName, std::move( shader ) ) );

	return bSuccess;
}

std::shared_ptr<SCION_RENDERING::Shader> AssetManager::GetShader( const std::string& shaderName )
{
	auto shaderItr = m_mapShader.find( shaderName );
	if ( shaderItr == m_mapShader.end() )
	{
		SCION_ERROR( "Failed to get shader [{0}] -- Does not exist!", shaderName );
		return nullptr;
	}

	return shaderItr->second;
}

bool AssetManager::AddMusic( const std::string& musicName, const std::string& filepath )
{
	if ( m_mapMusic.contains( musicName ) )
	{
		SCION_ERROR( "Failed to add music [{}] -- Already exists!", musicName );
		return false;
	}

	Mix_Music* music = Mix_LoadMUS( filepath.c_str() );

	if ( !music )
	{
		std::string error{ Mix_GetError() };
		SCION_ERROR( "Failed to load [{}] at path [{}] -- Mixer Error: {}", musicName, filepath, error );
		return false;
	}

	// Create the sound parameters
	SCION_SOUNDS::SoundParams params{ .name = musicName, .filename = filepath, .duration = Mix_MusicDuration( music ) };

	// Create the music Pointer
	auto musicPtr = std::make_shared<SCION_SOUNDS::Music>( params, MusicPtr{ music } );
	if ( !musicPtr )
	{
		SCION_ERROR( "Failed to create the music ptr for [{}]", musicName );
		return false;
	}

	auto [ itr, bSuccess ] = m_mapMusic.emplace( musicName, std::move( musicPtr ) );

	return bSuccess;
}

std::shared_ptr<SCION_SOUNDS::Music> AssetManager::GetMusic( const std::string& musicName )
{
	auto musicItr = m_mapMusic.find( musicName );
	if ( musicItr == m_mapMusic.end() )
	{
		SCION_ERROR( "Failed to get [{}] -- Does not exist!", musicName );
		return nullptr;
	}

	return musicItr->second;
}

bool AssetManager::AddSoundFx( const std::string& soundFxName, const std::string& filepath )
{
	if ( m_mapSoundFx.contains( soundFxName ) )
	{
		SCION_ERROR( "Failed to add soundfx [{}] -- Already exists!", soundFxName );
		return false;
	}

	Mix_Chunk* chunk = Mix_LoadWAV( filepath.c_str() );

	if ( !chunk )
	{
		std::string error{ Mix_GetError() };
		SCION_ERROR( "Failed to load [{}] at path [{}] -- Error: {}", soundFxName, filepath, error );
		return false;
	}

	SCION_SOUNDS::SoundParams params{ .name = soundFxName, .filename = filepath, .duration = chunk->alen / 179.4 };

	auto pSoundFx = std::make_shared<SCION_SOUNDS::SoundFX>( params, SoundFxPtr{ chunk } );
	auto [ itr, bSuccess ] = m_mapSoundFx.emplace( soundFxName, std::move( pSoundFx ) );

	return bSuccess;
}

std::shared_ptr<SCION_SOUNDS::SoundFX> AssetManager::GetSoundFx( const std::string& soundFxName )
{
	auto soundItr = m_mapSoundFx.find( soundFxName );
	if ( soundItr == m_mapSoundFx.end() )
	{
		SCION_ERROR( "Failed to get SoundFX [{}] -- Does Not exist!", soundFxName );
		return nullptr;
	}

	return soundItr->second;
}

bool AssetManager::AddPrefab( const std::string& sPrefabName, std::shared_ptr<SCION_CORE::Prefab> pPrefab )
{
	if (m_mapPrefabs.contains(sPrefabName))
	{
		SCION_ERROR( "Failed to add prefab [{}] -- Already exists in AssetManager.", sPrefabName );
		return false;
	}

	auto [ itr, bSuccess ] = m_mapPrefabs.emplace( sPrefabName, std::move( pPrefab ) );
	return bSuccess;
}

std::shared_ptr<SCION_CORE::Prefab> AssetManager::GetPrefab( const std::string& sPrefabName )
{
	auto prefabItr = m_mapPrefabs.find( sPrefabName );
	if ( prefabItr == m_mapPrefabs.end() )
	{
		SCION_ERROR( "Failed to get Prefab [{}] -- Does Not exist!", sPrefabName );
		return nullptr;
	}

	return prefabItr->second;
}

std::vector<std::string> AssetManager::GetAssetKeyNames( SCION_UTIL::AssetType eAssetType ) const
{
	switch ( eAssetType )
	{
	case SCION_UTIL::AssetType::TEXTURE:
		return SCION_UTIL::GetKeys( m_mapTextures, []( const auto& pair ) { return !pair.second->IsEditorTexture(); } );
	case SCION_UTIL::AssetType::FONT: return SCION_UTIL::GetKeys( m_mapFonts );
	case SCION_UTIL::AssetType::SOUNDFX: return SCION_UTIL::GetKeys( m_mapSoundFx );
	case SCION_UTIL::AssetType::MUSIC: return SCION_UTIL::GetKeys( m_mapMusic );
	case SCION_UTIL::AssetType::PREFAB: return SCION_UTIL::GetKeys( m_mapPrefabs );
	default: SCION_ASSERT( false && "Cannot get this type!" );
	}

	return std::vector<std::string>{};
}

bool AssetManager::ChangeAssetName( const std::string& sOldName, const std::string& sNewName,
									SCION_UTIL::AssetType eAssetType )
{
	switch ( eAssetType )
	{
	case SCION_UTIL::AssetType::TEXTURE: return SCION_UTIL::KeyChange( m_mapTextures, sOldName, sNewName );
	case SCION_UTIL::AssetType::FONT: return SCION_UTIL::KeyChange( m_mapFonts, sOldName, sNewName );
	case SCION_UTIL::AssetType::SOUNDFX: return SCION_UTIL::KeyChange( m_mapSoundFx, sOldName, sNewName );
	case SCION_UTIL::AssetType::MUSIC: return SCION_UTIL::KeyChange( m_mapMusic, sOldName, sNewName );
	default: SCION_ASSERT( false && "Cannot get this type!" );
	}

	return false;
}

bool AssetManager::CheckHasAsset( const std::string& sNameCheck, SCION_UTIL::AssetType eAssetType )
{
	switch ( eAssetType )
	{
	case SCION_UTIL::AssetType::TEXTURE: return m_mapTextures.contains( sNameCheck );
	case SCION_UTIL::AssetType::FONT: return m_mapFonts.contains( sNameCheck );
	case SCION_UTIL::AssetType::SOUNDFX: return m_mapSoundFx.contains( sNameCheck );
	case SCION_UTIL::AssetType::MUSIC: return m_mapMusic.contains( sNameCheck );
	case SCION_UTIL::AssetType::PREFAB: return m_mapPrefabs.contains( sNameCheck );
	default: SCION_ASSERT( false && "Cannot get this type!" );
	}

	return false;
}

bool AssetManager::DeleteAsset( const std::string& sAssetName, SCION_UTIL::AssetType eAssetType )
{
	switch ( eAssetType )
	{
	case SCION_UTIL::AssetType::TEXTURE:
		return std::erase_if( m_mapTextures, [ & ]( const auto& pair ) { return pair.first == sAssetName; } ) > 0;
	case SCION_UTIL::AssetType::FONT:
		return std::erase_if( m_mapFonts, [ & ]( const auto& pair ) { return pair.first == sAssetName; } ) > 0;
	case SCION_UTIL::AssetType::SOUNDFX:
		return std::erase_if( m_mapSoundFx, [ & ]( const auto& pair ) { return pair.first == sAssetName; } ) > 0;
	case SCION_UTIL::AssetType::MUSIC:
		return std::erase_if( m_mapMusic, [ & ]( const auto& pair ) { return pair.first == sAssetName; } ) > 0;
	case SCION_UTIL::AssetType::PREFAB: { // Prefabs contain files that must be cleaned up
		if (auto pPrefab = GetPrefab( sAssetName ))
		{
			if (!SCION_CORE::PrefabCreator::DeletePrefab(*pPrefab))
			{
				SCION_ERROR( "Failed to delete prefab [{}]", sAssetName );
				return false;
			}

			return m_mapPrefabs.erase( sAssetName ) > 0;
		}

		SCION_ERROR( "Failed to delete prefab [{}] - Does not exist in asset manager.", sAssetName );
		return false;
	}
	default: SCION_ASSERT( false && "Cannot get this type!" );
	}

	return false;
}

void AssetManager::CreateLuaAssetManager( sol::state& lua )
{
	auto& mainRegistry = MAIN_REGISTRY();
	auto& asset_manager = mainRegistry.GetAssetManager();

	lua.new_usertype<AssetManager>(
		"AssetManager",
		sol::no_constructor,
		"add_texture",
		sol::overload(
			[ & ]( const std::string& assetName, const std::string& filepath, bool pixel_art ) {
				return asset_manager.AddTexture( assetName, filepath, pixel_art, false );
			},
			[ & ]( const std::string& assetName, const std::string& filepath, bool pixel_art, bool bTileset ) {
				return asset_manager.AddTexture( assetName, filepath, pixel_art, bTileset );
			} ),
		"add_music",
		[ & ]( const std::string& musicName, const std::string& filepath ) {
			return asset_manager.AddMusic( musicName, filepath );
		},
		"add_soundfx",
		[ & ]( const std::string& soundFxName, const std::string& filepath ) {
			return asset_manager.AddSoundFx( soundFxName, filepath );
		},
		"add_font",
		[ & ]( const std::string& fontName, const std::string& fontPath, float fontSize ) {
			return asset_manager.AddFont( fontName, fontPath, fontSize );
		} );
}
} // namespace SCION_RESOURCES
