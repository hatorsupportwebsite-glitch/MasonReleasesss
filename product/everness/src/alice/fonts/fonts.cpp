//
// Created by rei on 12/2/2025.
//

#include "fonts.hpp"

#include "include/font_awesome/font_awesome.hpp"
#include "include/font_awesome/font_awesome_bytes.hpp"

#include "include/sf_compact/sf_compact_bold.hpp"
#include "include/sf_compact/sf_compact_medium.hpp"

#include "include/jetbrains_mono/jetbrains_mono_medium.hpp"

using alice::CFonts;
using alice::Font;

Font& CFonts::get( const std::string& key )
{
    return m_list[ key ];
}

void CFonts::init( )
{
    ImFontConfig icons_config;
    icons_config.FontDataOwnedByAtlas = false;
    icons_config.MergeMode = true;
    icons_config.SizePixels = 11;
    static constexpr ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0x0 };

    ImFontConfig main_cfg;
    main_cfg.FontDataOwnedByAtlas = false;
    // main_cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags::ImGuiFreeTypeBuilderFlags_Bold | ImGuiFreeTypeBuilderFlags::ImGuiFreeTypeBuilderFlags_LightHinting;

    ImGui::GetIO( ).Fonts->TexGlyphPadding = 1;

    // primary & ui
    m_list[ std::string( "primary" ) ] = { ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( fonts::sf_compact_medium_ttf, sizeof fonts::sf_compact_medium_ttf, 14.f * Font::kMaxFontScale, &main_cfg, ImGui::GetIO( ).Fonts->GetGlyphRangesCyrillic( ) ), 14 };
    m_list[ std::string( "icons" ) ] = { ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( font_awesome_binary, sizeof font_awesome_binary, 11 * Font::kMaxFontScale, &icons_config, icon_ranges ), 11 };

    m_list[ std::string( "small" ) ] = { ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( fonts::sf_compact_bold_ttf, sizeof fonts::sf_compact_bold_ttf, 10.f * Font::kMaxFontScale, &main_cfg, ImGui::GetIO( ).Fonts->GetGlyphRangesCyrillic( ) ), 20 };
    m_list[ std::string( "bold" ) ] = { ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( fonts::sf_compact_bold_ttf, sizeof fonts::sf_compact_bold_ttf, 14.f * Font::kMaxFontScale, &main_cfg, ImGui::GetIO( ).Fonts->GetGlyphRangesCyrillic( ) ), 14 };
    m_list[ std::string( "title" ) ] = { ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( fonts::sf_compact_bold_ttf, sizeof fonts::sf_compact_bold_ttf, 16.f * Font::kMaxFontScale, &main_cfg, ImGui::GetIO( ).Fonts->GetGlyphRangesCyrillic( ) ), 16 };
    m_list[ std::string( "large" ) ] = { ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( fonts::sf_compact_bold_ttf, sizeof fonts::sf_compact_bold_ttf, 20.f * Font::kMaxFontScale, &main_cfg, ImGui::GetIO( ).Fonts->GetGlyphRangesCyrillic( ) ), 20 };

    m_list[ std::string( "monospace" ) ] = { ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( fonts::jetbrains_mono_medium_ttf, sizeof fonts::jetbrains_mono_medium_ttf, 14.f * Font::kMaxFontScale, &main_cfg, ImGui::GetIO( ).Fonts->GetGlyphRangesCyrillic( ) ), 14 };
    ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( font_awesome_binary, sizeof font_awesome_binary, 11 * Font::kMaxFontScale, &icons_config, icon_ranges );

    ImGui::GetIO( ).FontGlobalScale = 1.f / Font::kMaxFontScale; // TODO
}
