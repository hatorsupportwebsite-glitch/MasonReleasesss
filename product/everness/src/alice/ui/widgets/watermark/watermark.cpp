//
// Created by Kai Tears on 23/01/2026.
//

#include "watermark.hpp"

#include "core.hpp"

#include "../../../fonts/fonts.hpp"
#include "../../../fonts/include/font_awesome/font_awesome.hpp"

#include "../../../animator/animator.hpp"
//#include "../../../blur/blur.hpp"

#include "../../../ui/ui.hpp"
#include "../../themes/themes.hpp"

#include <vector>
#include <chrono>
#include <ctime>
#include <cstdio>

using alice::CWatermark;

using namespace ImGui;

void CWatermark::render( bool allow )
{
    static CAnimator alpha { 0.f };
    static CAnimator drag_alpha { 0.f };

    alpha.update< float >( enabled ? 1.f : 0.f );
    auto const opacity = alpha.get< float >( );
    if ( opacity < 0.001f )
        return;

    PushStyleVar( ImGuiStyleVar_Alpha, opacity );

    auto const scale = GetStyle( ).Scale;

    PushFont( everness::ctx->alice->fonts->get( "monospace" ).ptr );

    auto const height = GetFontSize( ) + 20 * scale;
    ImVec2 const logo_size = { height, height };

    struct WatermarkDataItem
    {
        std::string icon;
        std::string info;
        ImColor color;
        float advance { 0 };
        std::function< void( ImDrawList*, const ImVec2& ) > advanceFunc { };
    };

    char time_buf[16] { };
    {
        const std::time_t tt = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now( ) );
        std::tm local_tm { };
#if defined( _WIN32 )
        localtime_s( &local_tm, &tt );
#else
        localtime_r( &tt, &local_tm );
#endif
        std::snprintf( time_buf, sizeof( time_buf ), "%02d:%02d:%02d", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec );
    }

    std::vector< WatermarkDataItem > const data = {
        { ICON_FA_USER, "Admin", ImColor( 125, 148, 250 ) },
        { ICON_FA_CLOCK, time_buf, ImColor( 250, 231, 125 ) } };

    float full_width { };
    for ( auto const& d : data )
    {
        full_width += CalcTextSize( d.icon.c_str( ) ).x + CalcTextSize( d.info.c_str( ) ).x + 15 * scale + d.advance;
    }

    PushStyleVar( ImGuiStyleVar_WindowPadding, { 0, 0 } );

    SetNextWindowSize( logo_size + ImVec2( full_width + 10 * scale, 0 ) );

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground;
    if ( !allow )
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;

    Begin( "##watermark", nullptr, flags );
    {
        ImRect const rect = {
            GetWindowPos( ),
            GetWindowPos( ) + GetWindowSize( ) };

        auto* draw = GetBackgroundDrawList( );

        bool const hovering = IsWindowHovered( );
        bool const dragging = IsMouseDragging( 0 );

        drag_alpha.update< float >( allow ? ( ( dragging && hovering ) ? 1.0f : ( hovering ? 0.5f : 0.f ) ) : 0.f );

        renderSelectionOutline( rect, drag_alpha.get< float >( ), 12 * scale );

        //g_blur.applyBlur( rect, draw, 10 * scale );

        draw->AddShadowRect( rect.Min + ImVec2( 4 * scale + logo_size.x, 4 * scale ), rect.Max - ImVec2( 4 * scale, 4 * scale ), Color( 0, 0, 0 ).modulate( opacity ), 42 * scale, { 0, 0 }, 0, 48 * scale );
        draw->AddRectFilled( rect.Min, rect.Max, Color( 20, 20, 20, 155 ).modulate( opacity ), 10 * scale );

        draw->AddRectFilled( rect.Min + ImVec2( logo_size.x, 0 ), rect.Max, Color( 20, 20, 20, 255 ).modulate( opacity ), 10 * scale );

        CUIElements::applyLiquidGlass( [ & ]( ImDrawList* draw ) -> void {
            draw->AddRect( rect.Min + ImVec2( logo_size.x, 0 ), rect.Max, Color( 32, 32, 32 ).modulate( opacity ), 10 * scale, 0, scale );
        },
                                       draw, { rect.Min + ImVec2( logo_size.x, 0 ), rect.Max }, Color( 32, 32, 32 ).u32( ), Color( 255, 255, 255 ).u32( ) );

        const int logo_start = draw->VtxBuffer.Size;
        draw->AddImage( CUserInterface::logo, rect.Min + ImVec2( 5 * scale, 5 * scale ), rect.Min + logo_size - ImVec2( 5 * scale, 5 * scale ), { 0, 0 }, { 1, 1 }, Color( 255, 255, 255 ).modulate( opacity ) );
        ShadeVertsLinearColorGradientKeepAlpha(draw, logo_start, draw->VtxBuffer.Size,
            rect.Min + ImVec2(5 * scale, 0), rect.Min + ImVec2(logo_size.x - 5 * scale, 0),
            IM_COL32_WHITE, Color(CThemes::current.colors.accent).u32());

        float offset { };
        for ( int it { 0 }; it < data.size( ); it++ )
        {
            auto& cur = data[ it ];

            draw->AddText( { rect.Min.x + logo_size.x + 10 * scale + offset, rect.Min.y + 10 * scale }, Color( cur.color.Value ).modulate( opacity ), cur.icon.c_str( ) );

            if ( cur.advanceFunc )
                cur.advanceFunc( draw, { rect.Min.x + logo_size.x + 10 * scale + offset + 5 * scale + CalcTextSize( cur.icon.c_str( ) ).x, rect.Min.y + 5 * scale } );

            draw->AddText( { rect.Min.x + logo_size.x + 10 * scale + offset + 5 * scale + CalcTextSize( cur.icon.c_str( ) ).x + cur.advance, rect.Min.y + 10 * scale }, Color( 255, 255, 255 ).modulate( opacity ), cur.info.c_str( ) );

            offset += CalcTextSize( cur.icon.c_str( ) ).x + CalcTextSize( cur.info.c_str( ) ).x + 15 * scale + cur.advance;
        }

        End( );
    }

    PopStyleVar( );

    PopFont( );

    PopStyleVar( );
}