//
// Created by rei on 2/4/2026.
//

#include "assets.hpp"

#include <filesystem>

#include "graphics/graphics.hpp"
#include "core.hpp"

using alice::CAssets;

std::unordered_map< std::string, ImTextureID > CAssets::m_loadedAssets { };

void CAssets::load( const std::string& name, const std::string& file )
{
    if ( !std::filesystem::exists( file ) )
        return;

    m_loadedAssets[ name ] = everness::ctx->graphics->loadTextureFromFile( file.c_str( ) );
}

void CAssets::loadFromFolder( const std::string& directory )
{
    std::filesystem::path const directory_path = directory;

    if ( !std::filesystem::exists( directory_path ) )
        return;

    if ( !std::filesystem::is_directory( directory_path ) )
        return;

    for ( const std::filesystem::directory_entry& entry :
          std::filesystem::directory_iterator(
              directory_path,
              std::filesystem::directory_options::skip_permission_denied ) )
    {
        if ( !entry.exists( ) )
            continue;

        if ( !entry.is_regular_file( ) )
            continue;

        m_loadedAssets[ entry.path( ).stem( ).string( ) ] = everness::ctx->graphics->loadTextureFromFile( entry.path( ).string( ).c_str( ) );
    }
}

ImTextureID CAssets::get( const std::string& name )
{
    return m_loadedAssets[ name ];
}