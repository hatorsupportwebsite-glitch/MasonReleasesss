//
// Created by Kai Tears on 27/01/2026.
//

#ifndef ENLIGHTENED_KEYBINDS_HPP
#define ENLIGHTENED_KEYBINDS_HPP

#include "imgui.h"
#include "imgui_internal.h"
#include "../widgets.hpp"
#include "animator/animator.hpp"

#include <optional>
#include <string>
#include <vector>

namespace alice
{
    struct KeyBindEntry
    {
        std::string name;
        std::optional< std::pair< int, int* > > value;

        bool enabled;
        bool removed;
    };

    struct KeyAnim
    {
        CAnimator alpha { 0.f };
        CAnimator y { 0.f };
        CAnimator value { ImVec4( 0, 0, 0, 0 ) };
        bool dying = false;
    };

    class CKeybinds : public CBaseWidget
    {
        ImVec2 calcWindowSize( float width, int amount );

    public:
        bool enabled = true;

        void render( bool allow, const std::vector< KeyBindEntry >& entries = { } );
    };
} // namespace alice

#endif // ENLIGHTENED_KEYBINDS_HPP
