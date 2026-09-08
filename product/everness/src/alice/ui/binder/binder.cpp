//
// Created by Kai Tears on 26/01/2026.
//

#include "binder.hpp"

#include "imgui.h"
#include "../popups/popups.hpp"

#include "fonts/include/font_awesome/font_awesome.hpp"

#include <algorithm>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

static int ImGuiKeyToVK( ImGuiKey key )
{
    if ( key >= ImGuiKey_A && key <= ImGuiKey_Z )
        return 'A' + ( key - ImGuiKey_A );
    if ( key >= ImGuiKey_0 && key <= ImGuiKey_9 )
        return '0' + ( key - ImGuiKey_0 );
    if ( key >= ImGuiKey_F1 && key <= ImGuiKey_F12 )
        return VK_F1 + ( key - ImGuiKey_F1 );
    if ( key >= ImGuiKey_Keypad0 && key <= ImGuiKey_Keypad9 )
        return VK_NUMPAD0 + ( key - ImGuiKey_Keypad0 );

    switch ( key )
    {
    case ImGuiKey_Tab:            return VK_TAB;
    case ImGuiKey_LeftArrow:      return VK_LEFT;
    case ImGuiKey_RightArrow:     return VK_RIGHT;
    case ImGuiKey_UpArrow:        return VK_UP;
    case ImGuiKey_DownArrow:      return VK_DOWN;
    case ImGuiKey_PageUp:         return VK_PRIOR;
    case ImGuiKey_PageDown:       return VK_NEXT;
    case ImGuiKey_Home:           return VK_HOME;
    case ImGuiKey_End:            return VK_END;
    case ImGuiKey_Insert:         return VK_INSERT;
    case ImGuiKey_Delete:         return VK_DELETE;
    case ImGuiKey_Backspace:      return VK_BACK;
    case ImGuiKey_Space:          return VK_SPACE;
    case ImGuiKey_Enter:          return VK_RETURN;
    case ImGuiKey_Escape:         return VK_ESCAPE;
    case ImGuiKey_LeftShift:      return VK_LSHIFT;
    case ImGuiKey_LeftCtrl:       return VK_LCONTROL;
    case ImGuiKey_LeftAlt:        return VK_LMENU;
    case ImGuiKey_RightShift:     return VK_RSHIFT;
    case ImGuiKey_RightCtrl:      return VK_RCONTROL;
    case ImGuiKey_RightAlt:       return VK_RMENU;
    case ImGuiKey_CapsLock:       return VK_CAPITAL;
    case ImGuiKey_ScrollLock:     return VK_SCROLL;
    case ImGuiKey_NumLock:        return VK_NUMLOCK;
    case ImGuiKey_PrintScreen:    return VK_SNAPSHOT;
    case ImGuiKey_Pause:          return VK_PAUSE;
    case ImGuiKey_MouseLeft:      return VK_LBUTTON;
    case ImGuiKey_MouseRight:     return VK_RBUTTON;
    case ImGuiKey_MouseMiddle:    return VK_MBUTTON;
    case ImGuiKey_MouseX1:        return VK_XBUTTON1;
    case ImGuiKey_MouseX2:        return VK_XBUTTON2;
    default: return 0;
    }
}

using alice::BindBool;
using alice::BindType;
using alice::CBinder;

CBinder::CBinder( ) = default;
CBinder::~CBinder( ) = default;

void CBinder::handleBool( BindBool& b )
{
    if ( b.key == ImGuiKey_None || !b.target || b.removed )
        return;

    int vk = ImGuiKeyToVK( b.key );
    if ( vk == 0 )
        return;

    bool key_down = ( GetAsyncKeyState( vk ) & 0x8000 ) != 0;

    switch ( b.type )
    {
    case BindType::Hold:
    {
        if ( key_down )
        {
            if ( !b.active )
            {
                *b.target = true;
                b.active = true;
            }
        } else
        {
            if ( b.active )
            {
                *b.target = false;
                b.active = false;
            }
        }
    }
    break;

    case BindType::Toggle:
    {
        if ( key_down && !b.active )
        {
            *b.target = !*b.target;
            b.active = true;
        }
        else if ( !key_down )
        {
            b.active = false;
        }
        break;
    }

    default:
        break;
    }
}

void CBinder::handleFloat( BindFloat& b )
{
    if ( !b.target || b.key == ImGuiKey_None || b.removed )
        return;

    if ( !b.active && *b.target != b.value )
        b.backup = *b.target;

    switch ( b.type )
    {
    case BindType::Hold:
    {
        if ( ImGui::IsKeyDown( b.key ) )
        {
            if ( !b.active )
            {
                b.backup = *b.target;
                *b.target = b.value;
                b.active = true;
            }
        } else if ( b.active )
        {
            *b.target = b.backup;
            b.active = false;
        }
        break;
    }

    case BindType::Toggle:
    {
        if ( ImGui::IsKeyPressed( b.key, false ) )
        {
            b.active = !b.active;

            if ( b.active )
            {
                b.backup = *b.target;
                *b.target = b.value;
            } else
            {
                *b.target = b.backup;
            }
        }
        break;
    }
    }
}

void CBinder::handleInt( BindInt& b )
{
    if ( !b.target || b.key == ImGuiKey_None || b.removed )
        return;

    if ( !b.active && *b.target != b.value )
        b.backup = *b.target;

    switch ( b.type )
    {
    case BindType::Hold:
    {
        if ( ImGui::IsKeyDown( b.key ) )
        {
            if ( !b.active )
            {
                b.backup = *b.target;
                *b.target = b.value;
                b.active = true;
            }
        } else if ( b.active )
        {
            *b.target = b.backup;
            b.active = false;
        }
        break;
    }

    case BindType::Toggle:
    {
        if ( ImGui::IsKeyPressed( b.key, false ) )
        {
            b.active = !b.active;

            if ( b.active )
            {
                b.backup = *b.target;
                *b.target = b.value;
            } else
            {
                *b.target = b.backup;
            }
        }
        break;
    }
    }
}

void CBinder::renderFloat( CPopup* binder, CPopup* bind_settings, const std::string& name, BindFloat** active_bind, std::vector< BindFloat >& list, float* value, float min_value, float max_value )
{
    if ( binder->begin( bind_settings->opened ) )
    {
        for ( int i = 0; i < list.size( ); ++i )
        {
            auto& b = list[ i ];

            ImGui::PushID( i );

            b.alpha.update< float >( b.removed ? 0.f : 1.f );

            ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle( ).Alpha * b.alpha.get< float >( ) );

            PopupEntries::itemHeightOverride = ( ImGui::GetFontSize( ) + 14 ) * b.alpha.get< float >( );
            if ( PopupEntries::button( ICON_FA_KEYBOARD, ( b.key == ImGuiKey_None ? "Unbound" : ImGui::GetKeyName( b.key ) ) ) && !bind_settings->opened && bind_settings->getAnimationValue( ) < 0.001f )
            {
                *active_bind = &b;
                bind_settings->opened = true;
            }
            PopupEntries::itemHeightOverride = std::nullopt;

            ImGui::PopStyleVar( );

            ImGui::PopID( );
        }

        struct _state
        {
            CAnimator separator { float( 0.f ) };
        };

        static std::unordered_map< uintptr_t, _state > __state;
        _state& state = __state[ reinterpret_cast< uintptr_t >( binder ) ];

        state.separator.update< float >( list.empty( ) ? 0.f : 1.f );

        ImGui::PushStyleVar( ImGuiStyleVar_Alpha, state.separator.get< float >( ) * ImGui::GetStyle( ).Alpha );
        PopupEntries::itemHeightOverride = state.separator.get< float >( ) * 21.f;
        PopupEntries::separator(  );
        PopupEntries::itemHeightOverride = std::nullopt;
        ImGui::PopStyleVar( );

        if ( PopupEntries::button( ICON_FA_FLOPPY_DISK, "Add hotkey" ) )
            list.push_back( BindFloat { .type = BindType::Hold, .target = value, .name = name, .value = *value, .min = min_value, .max = max_value } );

        PopupEntries::textColorOverride = ImColor( 191, 111, 105 );
        if ( PopupEntries::button( ICON_FA_REFRESH, "Reset all" ) )
            std::ranges::for_each( list, []( auto& bind ) { bind.removed = true; } );
        PopupEntries::textColorOverride = std::nullopt;

        binder->end( );
    }

    if ( bind_settings->begin( ) )
    {
        if ( *active_bind )
        {
            PopupEntries::bindListener(

                ICON_FA_KEYBOARD,
                "Key",
                &( *active_bind )->key,
                &( *active_bind )->listening );

            PopupEntries::bindType(

                ICON_FA_TOGGLE_ON,
                "Type",
                &( *active_bind )->type );

            PopupEntries::sliderFloat( ICON_FA_BARS, "Value", &( *active_bind )->value, min_value, max_value );

            PopupEntries::toggle( ICON_FA_EYE, "Visible", &( *active_bind )->visible );

            PopupEntries::separator( );

            PopupEntries::textColorOverride = ImColor( 191, 111, 105 );
            if ( PopupEntries::button( ICON_FA_TRASH, "Remove" ) )
            {
                ( *active_bind )->removed = true;
                bind_settings->opened = false;
            }
            PopupEntries::textColorOverride = std::nullopt;
        }

        bind_settings->end( );
    }
}

void CBinder::renderInt( CPopup* binder, CPopup* bind_settings, const std::string& name, BindInt** active_bind, std::vector< BindInt >& list, int* value, int min_value, int max_value )
{
    if ( binder->begin( bind_settings->opened ) )
    {
        for ( int i = 0; i < list.size( ); ++i )
        {
            auto& b = list[ i ];

            ImGui::PushID( i );

            b.alpha.update< float >( b.removed ? 0.f : 1.f );

            ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle( ).Alpha * b.alpha.get< float >( ) );

            PopupEntries::itemHeightOverride = ( ImGui::GetFontSize( ) + 14 ) * b.alpha.get< float >( );
            if ( PopupEntries::button( ICON_FA_KEYBOARD, ( b.key == ImGuiKey_None ? "Unbound" : ImGui::GetKeyName( b.key ) ) ) && !bind_settings->opened && bind_settings->getAnimationValue( ) < 0.001f )
            {
                *active_bind = &b;
                bind_settings->opened = true;
            }
            PopupEntries::itemHeightOverride = std::nullopt;

            ImGui::PopStyleVar( );

            ImGui::PopID( );
        }

        struct _state
        {
            CAnimator separator { float( 0.f ) };
        };

        static std::unordered_map< uintptr_t, _state > __state;
        _state& state = __state[ reinterpret_cast< uintptr_t >( binder ) ];

        state.separator.update< float >( list.empty( ) ? 0.f : 1.f );

        ImGui::PushStyleVar( ImGuiStyleVar_Alpha, state.separator.get< float >( ) * ImGui::GetStyle( ).Alpha );
        PopupEntries::itemHeightOverride = state.separator.get< float >( ) * 21.f;
        PopupEntries::separator( );
        PopupEntries::itemHeightOverride = std::nullopt;
        ImGui::PopStyleVar( );

        if ( PopupEntries::button( ICON_FA_FLOPPY_DISK, "Add hotkey" ) )
            list.push_back( BindInt { .type = BindType::Hold, .target = value, .name = name, .value = *value, .min = min_value, .max = max_value } );

        PopupEntries::textColorOverride = ImColor( 191, 111, 105 );
        if ( PopupEntries::button( ICON_FA_REFRESH, "Reset all" ) )
            std::ranges::for_each( list, []( auto& bind ) { bind.removed = true; } );
        PopupEntries::textColorOverride = std::nullopt;

        binder->end( );
    }

    if ( bind_settings->begin( ) )
    {
        if ( *active_bind )
        {
            PopupEntries::bindListener(

                ICON_FA_KEYBOARD,
                "Key",
                &( *active_bind )->key,
                &( *active_bind )->listening );

            PopupEntries::bindType(

                ICON_FA_TOGGLE_ON,
                "Type",
                &( *active_bind )->type );

            PopupEntries::sliderInt( ICON_FA_BARS, "Value", &( *active_bind )->value, min_value, max_value );

            PopupEntries::toggle( ICON_FA_EYE, "Visible", &( *active_bind )->visible );

            PopupEntries::separator( );

            PopupEntries::textColorOverride = ImColor( 191, 111, 105 );
            if ( PopupEntries::button( ICON_FA_TRASH, "Remove" ) )
            {
                ( *active_bind )->removed = true;
                bind_settings->opened = false;
            }
            PopupEntries::textColorOverride = std::nullopt;
        }

        bind_settings->end( );
    }
}

void CBinder::renderBool( CPopup* binder, CPopup* bind_settings, const std::string& name, BindBool** active_bind, std::vector< BindBool >& list, bool* value )
{
    if ( binder->begin( bind_settings->opened ) )
    {
        for ( int i = 0; i < list.size( ); ++i )
        {
            auto& b = list[ i ];

            ImGui::PushID( i );

            b.alpha.update< float >( b.removed ? 0.f : 1.f );

            ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle( ).Alpha * b.alpha.get< float >( ) );

            PopupEntries::itemHeightOverride = ( ImGui::GetFontSize( ) + 14 ) * b.alpha.get< float >( );
            if ( PopupEntries::button( ICON_FA_KEYBOARD, ( b.key == ImGuiKey_None ? "Unbound" : ImGui::GetKeyName( b.key ) ) ) && !bind_settings->opened && bind_settings->getAnimationValue( ) < 0.001f )
            {
                *active_bind = &b;
                bind_settings->opened = true;
            }
            PopupEntries::itemHeightOverride = std::nullopt;

            ImGui::PopStyleVar( );

            ImGui::PopID( );
        }

        struct _state
        {
            CAnimator separator { float( 0.f ) };
        };

        static std::unordered_map< uintptr_t, _state > __state;
        _state& state = __state[ reinterpret_cast< uintptr_t >( binder ) ];

        state.separator.update< float >( list.empty( ) ? 0.f : 1.f );

        ImGui::PushStyleVar( ImGuiStyleVar_Alpha, state.separator.get< float >( ) * ImGui::GetStyle( ).Alpha );
        PopupEntries::itemHeightOverride = state.separator.get< float >( ) * 21.f;
        PopupEntries::separator( );
        PopupEntries::itemHeightOverride = std::nullopt;
        ImGui::PopStyleVar( );

        if ( PopupEntries::button( ICON_FA_FLOPPY_DISK, "Add hotkey" ) )
            list.push_back( BindBool { .type = BindType::Hold, .target = value, .name = name } );

        PopupEntries::textColorOverride = ImColor( 191, 111, 105 );
        if ( PopupEntries::button( ICON_FA_REFRESH, "Reset all" ) )
            std::ranges::for_each( list, []( auto& bind ) { bind.removed = true; } );
        PopupEntries::textColorOverride = std::nullopt;

        binder->end( );
    }

    if ( bind_settings->begin( ) )
    {
        if ( *active_bind )
        {
            PopupEntries::bindListener(

                ICON_FA_KEYBOARD,
                "Key",
                &( *active_bind )->key,
                &( *active_bind )->listening );

            PopupEntries::bindType(

                ICON_FA_TOGGLE_ON,
                "Type",
                &( *active_bind )->type );

            PopupEntries::toggle( ICON_FA_EYE, "Visible", &( *active_bind )->visible );

            PopupEntries::separator( );

            PopupEntries::textColorOverride = ImColor( 191, 111, 105 );
            if ( PopupEntries::button( ICON_FA_TRASH, "Remove" ) )
            {
                ( *active_bind )->removed = true;
                bind_settings->opened = false;
            }
            PopupEntries::textColorOverride = std::nullopt;
        }

        bind_settings->end( );
    }
}
