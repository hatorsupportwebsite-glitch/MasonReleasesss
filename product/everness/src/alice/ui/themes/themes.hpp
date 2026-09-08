//
// Created by rei on 1/28/2026.
//

#ifndef ENLIGHTENED_DX9_THEMES_HPP
#define ENLIGHTENED_DX9_THEMES_HPP

#include "imgui.h"

#include <unordered_map>
#include <string>

namespace alice
{
    struct Theme
    {
        struct
        {
            ImVec4 text;
            ImVec4 textHover;
            ImVec4 textDim;
            ImVec4 textUnsafe;

            ImVec4 border;

            ImVec4 accent;

            struct
            {
                ImVec4 textCategory;

                ImVec4 background;
                ImVec4 backgroundHovered;
                ImVec4 backgroundActive;
                ImVec4 backgroundClicked;
            } tabs;

            struct
            {
                ImVec4 background;
                ImVec4 grab;
            } scrollbar;

            struct
            {
                ImVec4 background;
                ImVec4 overlay;
                ImVec4 overlayOutline;
                ImVec4 overlayShadow;
                ImVec4 shadow;
                ImVec4 settingsOverlay;
            } window;

            struct
            {
                ImVec4 background;
                ImVec4 backgroundHovered;
                ImVec4 backgroundClicked;
                ImVec4 backgroundOverlay;
                ImVec4 backgroundOverlayHovered;
                ImVec4 backgroundOverlayActive;
                ImVec4 colorPickerSelect;
                ImVec4 outline;
                ImVec4 overlayText;
            } elem;
        } colors;

        struct
        {
            int rounding;
            ImVec2 padding;
            bool animatePadding;
            bool spaceIcons;

            struct
            {
                ImColor text;
                ImColor background;
                ImColor outline;
                ImColor shadow;
                ImVec4 overlay;
                ImVec4 overlayHovered;
                ImVec4 overlayActive;
                ImVec4 sliderBackground;
            } colors;

            struct
            {
                int size;
            } shadow;
        } popup;
    };

    class CThemes
    {
        std::unordered_map< std::string, Theme > m_themes { };

        static Theme m_default;
        static Theme m_solid;
        static Theme m_white;

    public:
        CThemes( ) = default;
        ~CThemes( ) = default;

        static Theme current;

        void addDefaults( );

        std::unordered_map< std::string, Theme >& getAllThemes( );

        Theme getTheme( const std::string& key );
        void addTheme( const std::string& key, const Theme& theme );
    };
} // namespace alice

#endif // ENLIGHTENED_DX9_THEMES_HPP
