//
// Created by rei on 12/2/2025.
//

#ifndef EVERNESS_UI_FONTS_HPP
#define EVERNESS_UI_FONTS_HPP

#include <unordered_map>
#include <string>

#include "imgui.h"

namespace alice
{
    struct Font
    {
        ImFont* ptr;
        int size;

        static constexpr int kMaxFontScale = 4;
    };

    class CFonts
    {
        std::unordered_map< std::string, Font > m_list;

    public:
        ~CFonts( ) = default;
        CFonts( ) = default;

        void init( );

        Font& get( const std::string& key );
    };
} // namespace alice

#endif // EVERNESS_UI_FONTS_HPP
