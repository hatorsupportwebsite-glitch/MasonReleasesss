//
// Created by rei on 1/28/2026.
//

#include "themes.hpp"

using alice::CThemes;
using alice::Theme;

Theme CThemes::m_default {
    .colors = {
        .text = ImColor( 255, 255, 255, 200 ).Value,
        .textHover = ImColor( 200, 200, 200 ).Value,
        .textDim = ImColor( 155, 155, 155 ).Value,
        .textUnsafe = ImColor( 255, 199, 56 ).Value,

        .border = ImColor( 24, 24, 24, 200 ).Value,

        .accent = ImColor( 120, 255, 100 ).Value,

        .tabs = {
            .textCategory = ImColor( 1.f, 1.f, 1.f, 0.5f ).Value,

            .background = ImColor( 255, 255, 255, 0 ).Value,
            .backgroundHovered = ImColor( 255, 255, 255, 4 ).Value,
            .backgroundActive = ImColor( 255, 255, 255, 8 ).Value,
            .backgroundClicked = ImColor( 255, 255, 255, 16 ).Value,
        },

        .scrollbar {
            .background = { 0, 0, 0, 0 },
            .grab = ImColor( 255, 255, 255, 5 ).Value,
        },

        .window = {
            .background = ImColor( 12, 12, 12, 240 ).Value,
            .overlayOutline = ImColor( 24, 24, 24, 200 ).Value,
            .overlayShadow = ImColor( 0, 0, 0, 100 ).Value,
            .shadow = ImColor( 0, 0, 0, 160 ).Value,
            .settingsOverlay = ImColor( 8, 8, 8, 220 ).Value,
        },

        .elem = {
            .background = ImColor( 40, 40, 40, 40 ).Value,
            .backgroundHovered = ImColor( 40, 40, 40, 80 ).Value,
            .backgroundClicked = ImColor( 42, 42, 42, 100 ).Value,
            .backgroundOverlay = ImColor( 255, 255, 255, 5 ).Value,
            .backgroundOverlayHovered = ImColor( 255, 255, 255, 10 ).Value,
            .backgroundOverlayActive = ImColor( 255, 255, 255, 15 ).Value,
            .colorPickerSelect = ImColor( 255, 255, 255 ).Value,
            .outline = ImColor( 42, 42, 42, 50 ).Value,
            .overlayText = ImColor( 255, 255, 255, 96 ).Value,
        },
    },

    .popup = {
        .rounding = 16,
        .padding = { 8, 8 },
        .animatePadding = false,
        .spaceIcons = false,

        .colors = { .text = ImColor( 255, 255, 255, 185 ), .background = ImColor( 20, 20, 20, 140 ).Value, .outline = ImColor( 255, 255, 255, 8 ), .shadow = ImColor( 0, 0, 0, 140 ), .overlay = ImColor( 255, 255, 255, 0 ), .overlayHovered = ImColor( 255, 255, 255, 8 ), .overlayActive = ImColor( 255, 255, 255, 16 ), .sliderBackground = ImColor( 0, 0, 0, 100 ) },

        .shadow = { .size = 30 },
    },
};

Theme CThemes::m_solid {
    .colors = {
        .text = ImColor( 255, 255, 255, 200 ).Value,
        .textHover = ImColor( 200, 200, 200 ).Value,
        .textDim = ImColor( 155, 155, 155 ).Value,
        .textUnsafe = ImColor( 255, 199, 56 ).Value,

        .border = ImColor( 25, 25, 25 ).Value,

        .accent = ImColor( 120, 255, 100 ).Value,

        .tabs = {
            .textCategory = ImColor( 1.f, 1.f, 1.f, 0.5f ).Value,

            .background = ImColor( 255, 255, 255, 0 ).Value,
            .backgroundHovered = ImColor( 255, 255, 255, 4 ).Value,
            .backgroundActive = ImColor( 255, 255, 255, 8 ).Value,
            .backgroundClicked = ImColor( 255, 255, 255, 16 ).Value,
        },

        .scrollbar {
            .background = { 0, 0, 0, 0 },
            .grab = ImColor( 255, 255, 255, 5 ).Value,
        },

        .window = {
            .background = ImColor( 12, 12, 12, 255 ).Value,
            .overlayOutline = ImColor( 24, 24, 24, 200 ).Value,
            .overlayShadow = ImColor( 0, 0, 0, 100 ).Value,
            .shadow = ImColor( 0, 0, 0, 160 ).Value,
            .settingsOverlay = ImColor( 8, 8, 8, 220 ).Value,
        },

        .elem = {
            .background = ImColor( 14, 14, 14 ).Value,
            .backgroundHovered = ImColor( 16, 16, 16 ).Value,
            .backgroundClicked = ImColor( 18, 18, 18 ).Value,
            .backgroundOverlay = ImColor( 255, 255, 255, 4 ).Value,
            .backgroundOverlayHovered = ImColor( 255, 255, 255, 8 ).Value,
            .backgroundOverlayActive = ImColor( 255, 255, 255, 14 ).Value,
            .colorPickerSelect = ImColor( 255, 255, 255 ).Value,
            .outline = ImColor( 20, 20, 20 ).Value,
            .overlayText = ImColor( 255, 255, 255, 96 ).Value,
        },

    },

    .popup = {
        .rounding = 16,
        .padding = { 8, 8 },
        .animatePadding = false,
        .spaceIcons = false,

        .colors = { .text = ImColor( 255, 255, 255, 185 ), .background = ImColor( 14, 14, 14, 255 ).Value, .outline = ImColor( 255, 255, 255, 8 ), .shadow = ImColor( 0, 0, 0, 140 ), .overlay = ImColor( 255, 255, 255, 0 ), .overlayHovered = ImColor( 255, 255, 255, 8 ), .overlayActive = ImColor( 255, 255, 255, 16 ), .sliderBackground = ImColor( 0, 0, 0, 100 ) },

        .shadow = { .size = 30 },
    },
};

Theme CThemes::m_white {
    .colors = {
        .text = ImColor( 20, 20, 20, 220 ).Value,
        .textHover = ImColor( 20, 20, 20, 255 ).Value,
        .textDim = ImColor( 60, 60, 60, 180 ).Value,
        .textUnsafe = ImColor( 176, 140, 46 ).Value,

        .border = ImColor( 220, 220, 220 ).Value,

        .accent = ImColor( 90, 130, 255 ).Value,

        .tabs = {
            .textCategory = ImColor( 0.f, 0.f, 0.f, 0.45f ).Value,

            .background = ImColor( 0, 0, 0, 0 ).Value,
            .backgroundHovered = ImColor( 0, 0, 0, 4 ).Value,
            .backgroundActive = ImColor( 0, 0, 0, 8 ).Value,
            .backgroundClicked = ImColor( 0, 0, 0, 14 ).Value,
        },

        .scrollbar = {
            .background = ImColor( 0, 0, 0, 0 ).Value,
            .grab = ImColor( 0, 0, 0, 10 ).Value,
        },

        .window = {
            .background = ImColor( 248, 248, 248, 255 ).Value,
            .overlayOutline = ImColor( 0, 0, 0, 25 ).Value,
            .overlayShadow = ImColor( 0, 0, 0, 40 ).Value,
            .shadow = ImColor( 0, 0, 0, 60 ).Value,
            .settingsOverlay = ImColor( 240, 240, 240, 240 ).Value,
        },

        .elem = {
            .background = ImColor( 245, 245, 245 ).Value,
            .backgroundHovered = ImColor( 243, 243, 243 ).Value,
            .backgroundClicked = ImColor( 240, 240, 240 ).Value,

            .backgroundOverlay = ImColor( 0, 0, 0, 4 ).Value,
            .backgroundOverlayHovered = ImColor( 0, 0, 0, 8 ).Value,
            .backgroundOverlayActive = ImColor( 0, 0, 0, 14 ).Value,
            .colorPickerSelect = ImColor( 64, 64, 64 ).Value,

            .outline = ImColor( 220, 220, 220 ).Value,
            .overlayText = ImColor( 0, 0, 0, 110 ).Value,
        },
    },

    .popup = {
        .rounding = 16,
        .padding = { 8, 8 },
        .animatePadding = false,
        .spaceIcons = false,

        .colors = {
            .text = ImColor( 20, 20, 20, 220 ),
            .background = ImColor( 248, 248, 248, 255 ).Value,
            .outline = ImColor( 0, 0, 0, 20 ),
            .shadow = ImColor( 255, 255, 255, 60 ),

            .overlay = ImColor( 0, 0, 0, 0 ),
            .overlayHovered = ImColor( 0, 0, 0, 8 ),
            .overlayActive = ImColor( 0, 0, 0, 14 ),

            .sliderBackground = ImColor( 0, 0, 0, 10 ),
        },

        .shadow = { .size = 24 },
    },
};

Theme CThemes::current { m_default };

void CThemes::addDefaults( )
{
    m_themes[ "Default" ] = m_default;
    m_themes[ "Solid" ] = m_solid;
    m_themes[ "Solid White" ] = m_white;
}

std::unordered_map< std::string, Theme >& CThemes::getAllThemes( )
{
    return m_themes;
}

Theme CThemes::getTheme( const std::string& key )
{
    return m_themes[ key ];
}

void CThemes::addTheme( const std::string& key, const Theme& theme )
{
    m_themes[ key ] = theme;
}
