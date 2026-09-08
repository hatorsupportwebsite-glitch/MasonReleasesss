//
// Created by Kai Tears on 27/01/2026.
//

#include "keybinds.hpp"

#include "../../../fonts/fonts.hpp"
#include "../../../fonts/include/font_awesome/font_awesome.hpp"

#include "../../../animator/animator.hpp"
//#include "../../../blur/blur.hpp"

#include "core.hpp"

#include "../../../ui/ui.hpp"

#include <unordered_set>

using alice::CKeybinds;

using namespace ImGui;

ImVec2 CKeybinds::calcWindowSize( float width, int amount )
{
    auto const height = GetFontSize( ) + 20.f * GetStyle( ).Scale;
    float const total_height = ( amount + 1 ) * height + ( 10 * amount );

    float const pad_x = 20.f * GetStyle( ).Scale;

    return {
        width + pad_x * 2,
        total_height };
}

void CKeybinds::render( bool allow, const std::vector< KeyBindEntry >& entries )
{
    static CAnimator alpha { 0.f };
    static CAnimator drag_alpha { 0.f };

    static std::unordered_map< std::string, KeyAnim > anims;

    alpha.update< float >( enabled ? 1.f : 0.f );
    auto const opacity = alpha.get< float >( );
    if ( opacity < 0.001f )
        return;

    std::unordered_set< std::string > alive;

    for ( auto& e : entries )
        alive.insert( e.name );

    float layout_y = 0.f;
    float row_h = 30.f * GetStyle( ).Scale + GetFontSize( );

    for ( auto& e : entries )
    {
        auto& anim = anims[ e.name ];

        bool should_show = e.enabled && !e.removed;

        auto const visible = e.enabled && !e.removed;

        anim.alpha.update< float >( should_show ? 1.f : 0.f, 0.15f );
        if ( visible )
        {
            anim.y.update< float >( layout_y, 0.2f );
            layout_y += row_h;
        }

        anim.dying = !should_show;
    }

    for ( auto it = anims.begin( ); it != anims.end( ); )
    {
        if ( it->second.dying && it->second.alpha.get< float >( ) < 0.01f )
            it = anims.erase( it );
        else
            ++it;
    }

    PushStyleVar( ImGuiStyleVar_Alpha, opacity );

    PushFont( everness::ctx->alice->fonts->get( "monospace" ).ptr );

    PushStyleColor( ImGuiCol_WindowShadow, { 0, 0, 0, 0 } );
    PushStyleVar( ImGuiStyleVar_WindowPadding, { 0, 0 } );

    static CAnimator win_size { ImVec2( 0, 0 ) };

    win_size.update( calcWindowSize( [ & ]( ) -> float {
        float width = 100.f;

        for ( auto& entry : entries )
        {
            if ( !entry.enabled )
                continue;

            float w = CalcTextSize( entry.name.c_str() ).x;

            if ( entry.value.has_value( ) )
            {
                std::string const val = std::to_string( *( *entry.value ).second );
                w += 15.f + CalcTextSize( val.c_str( ) ).x;
            }

            width = std::max( width, w + 20.f );
        }

        return width; }( ),
                                     [ & ] {
                                         int result { 0 };

                                         for ( auto& it : entries )
                                         {
                                             if ( it.enabled )
                                                 result++;
                                         }

                                         return result;
                                     }( ) ) );

    SetNextWindowSize( win_size.get< ImVec2 >( ) );

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground;
    if ( !allow )
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;

    Begin( "##binds", 0, flags );

    auto const scale = GetStyle( ).Scale;

    auto const window = GetCurrentWindow( );

    bool const hovering = IsWindowHovered( );
    bool const dragging = IsMouseDragging( 0 );

    drag_alpha.update< float >( allow ? ( ( dragging && hovering ) ? 1.0f : ( hovering ? 0.5f : 0.f ) ) : 0.f );

    renderSelectionOutline( { window->Rect( ).Min + ImVec2( 7, -4 ), window->Rect( ).Max - ImVec2( 7, -5 ) }, drag_alpha.get< float >( ), 12 * scale );

    {
        auto const text_sz = CalcTextSize( ICON_FA_KEYBOARD " Keybinds" );

        ImRect const rect = {
            { ( float ) int( window->Pos.x + 10 ), ( float ) int( window->Pos.y ) },
            { ( float ) int( window->Pos.x + window->Size.x - 10 ), ( float ) int( window->Pos.y + 20 * GetStyle( ).Scale + text_sz.y ) } };

        //g_blur.applyBlur( rect, GetBackgroundDrawList( ), 10 * scale, 1.f, opacity );

        GetBackgroundDrawList( )->AddShadowRect( rect.Min + ImVec2( 6 * GetStyle( ).Scale, 6 * GetStyle( ).Scale ), rect.Max - ImVec2( 6 * GetStyle( ).Scale, 6 * GetStyle( ).Scale ), Color( 0, 0, 0 ).modulate( opacity ), 32 * GetStyle( ).Scale, { 0, 0 }, 0, 64 * GetStyle( ).Scale );

        GetBackgroundDrawList( )->AddRectFilled( rect.Min, rect.Max, Color( 20, 20, 20, 255 ).modulate( opacity ), 10 * GetStyle( ).Scale );
        // GetBackgroundDrawList( )->AddRect( rect.Min, rect.Max, Color( 255, 255, 255, 5 ).modulate( log.alpha.get< float >( ) ), 10 );

        CUIElements::applyLiquidGlass( [ & ]( ImDrawList* draw ) -> void {
            draw->AddRect( rect.Min, rect.Max, Color( 32, 32, 32 ).modulate( opacity ), 10 * GetStyle( ).Scale, 0, GetStyle( ).Scale );
        },
                                       GetBackgroundDrawList( ), { rect.Min, rect.Max }, Color( 32, 32, 32 ).u32( ), Color( 255, 255, 255 ).u32( ) );

        GetBackgroundDrawList( )->AddText( { window->Pos.x + window->Size.x / 2 - text_sz.x / 2, window->Pos.y + 10 }, Color( 255, 255, 255 ).modulate( opacity ), ICON_FA_KEYBOARD " Keybinds" );
    }

    for ( int it { 0 }; it < entries.size( ); it++ )
    {
        auto& entry = entries[ it ];

        auto itt = anims.find( entry.name );
        if ( itt == anims.end( ) )
            continue;

        auto& anim = itt->second;

        auto const calc_alpha = opacity * anim.alpha.get< float >( );

        auto const text_sz = CalcTextSize( entry.name.c_str( ) );

        auto const log_y =
            window->Pos.y +
            anim.y.get< float >( );

        ImRect const rect = {
            { ( float ) int( window->Pos.x + 10 * GetStyle( ).Scale ), ( float ) int( log_y - 0 * GetStyle( ).Scale + ( 20 * GetStyle( ).Scale + text_sz.y + 10 ) ) },
            { ( float ) int( window->Pos.x + window->Size.x - 10 * GetStyle( ).Scale ), ( float ) int( log_y + 20 * GetStyle( ).Scale + text_sz.y + ( 20 * GetStyle( ).Scale + text_sz.y + 10 ) ) } };

        //g_blur.applyBlur( rect, GetBackgroundDrawList( ), 10 * scale, 1.f, calc_alpha );

        GetBackgroundDrawList( )->AddShadowRect( rect.Min + ImVec2( 6 * GetStyle( ).Scale, 6 * GetStyle( ).Scale ), rect.Max - ImVec2( 6 * GetStyle( ).Scale, 6 * GetStyle( ).Scale ), Color( 0, 0, 0 ).modulate( calc_alpha ), 32 * GetStyle( ).Scale, { 0, 0 }, 0, 64 * GetStyle( ).Scale );

        GetBackgroundDrawList( )->AddRectFilled( rect.Min, rect.Max, Color( 20, 20, 20, 155 ).modulate( calc_alpha ), 10 * GetStyle( ).Scale );

        if ( !entry.value.has_value( ) )
            GetBackgroundDrawList( )->AddText( { window->Pos.x + window->Size.x / 2 - text_sz.x / 2, log_y + 10 + ( 20 * GetStyle( ).Scale + text_sz.y + 10 ) }, Color( 255, 255, 255 ).modulate( calc_alpha ), entry.name.c_str( ) );
        else
        {
            GetBackgroundDrawList( )->AddText( { window->Pos.x + 20, log_y + 10 + ( 20 * GetStyle( ).Scale + text_sz.y + 10 ) }, Color( 255, 255, 255 ).modulate( calc_alpha ), entry.name.c_str( ) );

            std::string const val = std::to_string( *( *entry.value ).second );
            anim.value.update< ImVec4 >( ( ( *entry.value ).first == *( *entry.value ).second ) ? ImColor( 255, 255, 255 ).Value : ImColor( 155, 155, 155 ).Value );
            GetBackgroundDrawList( )->AddText( { window->Pos.x + window->Size.x - 20 - CalcTextSize( val.c_str( ) ).x, log_y + 10 + ( 20 * GetStyle( ).Scale + text_sz.y + 10 ) }, Color( anim.value.get< ImVec4 >( ) ).modulate( calc_alpha ), val.c_str( ) );
        }
    }

    End( );

    PopStyleVar( );
    PopStyleColor( );

    PopFont( );

    PopStyleVar( );
}