//
// Created by Kai Tears on 26/01/2026.
//

#ifndef ENLIGHTENED_BINDER_HPP
#define ENLIGHTENED_BINDER_HPP

#include "imgui.h"
#include "animator/animator.hpp"

namespace alice
{
    enum class BindType
    {
        Toggle,
        Hold
    };

    struct BindBool
    {
        BindType type;
        bool* target;
        std::string name;

        ImGuiKey key = ImGuiKey_None;

        bool ownsTarget = false;

        bool listening = false;
        bool active = false;
        bool visible = false;

        // ui things
        CAnimator alpha { 0.f };
        bool removed { false };
    };

    struct BindFloat
    {
        BindType type;
        float* target = nullptr;
        std::string name;

        float value = 0.f;
        float backup = 0.f;

        ImGuiKey key = ImGuiKey_None;

        bool ownsTarget = false;

        bool listening = false;
        bool active = false;
        bool visible = false;

        float min;
        float max;

        // ui
        CAnimator alpha { 0.f };
        bool removed = false;
    };

    struct BindInt
    {
        BindType type;
        int* target = nullptr;
        std::string name;

        int value = 0;
        int backup = 0;

        ImGuiKey key = ImGuiKey_None;

        bool ownsTarget = false;

        bool listening = false;
        bool active = false;
        bool visible = false;

        int min;
        int max;

        // ui
        CAnimator alpha { 0.f };
        bool removed = false;
    };

    class CPopup;

    class CBinder
    {
    public:
        CBinder( );
        ~CBinder( );

        void handleBool( BindBool& bind );
        void handleFloat( BindFloat& b );
        void handleInt( BindInt& b );

        void renderFloat( CPopup* binder, CPopup* bind_settings, const std::string& name, BindFloat** active_bind, std::vector< BindFloat >& list, float* value, float min_value, float max_value );
        void renderInt( CPopup* binder, CPopup* bind_settings, const std::string& name, BindInt** active_bind, std::vector< BindInt >& list, int* value, int min_value, int max_value );
        void renderBool( CPopup* binder, CPopup* bind_settings, const std::string& name, BindBool** active_bind, std::vector< BindBool >& list, bool* value );
    };
} // namespace alice

#endif // ENLIGHTENED_BINDER_HPP
