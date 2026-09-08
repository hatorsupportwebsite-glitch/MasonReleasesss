//
// Created by Kai Tears on 23/01/2026.
//

#ifndef ENLIGHTENED_WIDGETS_HPP
#define ENLIGHTENED_WIDGETS_HPP

#include "imgui.h"
#include "imgui_internal.h"

#include <memory>

namespace alice
{
    class CWatermark;
    class CHitLogs;
    class CKeybinds;

    class CBaseWidget
    {
    public:
        void renderSelectionOutline( const ImRect& rect, float alpha, float rounding );

        ImVec2 m_savedPosition;
    };

    class CWidgets
    {
    public:
        CWidgets( );
        ~CWidgets( );

        std::unique_ptr< CWatermark > watermark;
        std::unique_ptr< CHitLogs > hitLogs;
        std::unique_ptr< CKeybinds > keybinds;

        void render( );
    };
} // namespace alice

#endif // ENLIGHTENED_WIDGETS_HPP
