//
// Created by rei on 1/27/2026.
//

#ifndef ENLIGHTENED_ALERTS_HPP
#define ENLIGHTENED_ALERTS_HPP

#include "imgui.h"
#include "animator/animator.hpp"

#include <functional>

struct ImGuiWindow;

namespace alice
{
    struct Alert
    {
        ImVec2 size;
        std::function< void( Alert& alert ) > body;
        std::function< void( Alert& alert ) > task;

        bool erase = false;
        CAnimator alpha { float( 0.f ) };

        void close( )
        {
            erase = true;
        }
    };

    class CAlerts
    {
        std::unordered_map< std::string, Alert > m_alerts;

        void initStyle( const std::string& title, const Alert& alert, ImGuiWindow* window );

    public:
        ~CAlerts( ) = default;
        CAlerts( ) = default;

        static bool button( const std::string& label, const ImVec2& size, bool primary = false );
        static void renderContents( ImVec2 pos, const char* text, const char* text_end, float wrap_width );

        void create( const std::string& title, const ImVec2& size, const std::function< void( Alert& alert ) >& body, const std::function< void( Alert& alert ) >& task = nullptr );
        void render( );

        [[nodiscard]] bool find( const std::string& title ) const;
    };
} // namespace alice

#endif // ENLIGHTENED_ALERTS_HPP
