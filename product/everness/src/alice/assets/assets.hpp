//
// Created by rei on 2/4/2026.
//

#ifndef SCATTEREDABYSS_DESKTOP_ASSETS_HPP
#define SCATTEREDABYSS_DESKTOP_ASSETS_HPP

#include "imgui.h"

#include <string>
#include <unordered_map>

namespace alice
{
    class CAssets
    {
        static std::unordered_map< std::string, ImTextureID > m_loadedAssets;

    public:
        void load( const std::string& name, const std::string& file );
        void loadFromFolder( const std::string& directory );
        ImTextureID get( const std::string& name );
    };
} // namespace alice

#endif // SCATTEREDABYSS_DESKTOP_ASSETS_HPP
