//
// Created by rei on 1/27/2026.
//

#include "alerts.hpp"

// #include "../../blur/blur.hpp"
#include "../../fonts/fonts.hpp"
#include "../elem/elem.hpp"
#include "../themes/themes.hpp"
#include "core.hpp"

using namespace ImGui;

using alice::Alert;
using alice::CAlerts;

void CAlerts::create( const std::string& title, const ImVec2& size, const std::function< void( Alert& alert ) >& body, const std::function< void( Alert& alert ) >& task )
{
    m_alerts[ title ] = {
        .size = size,
        .body = body,
        .task = task };
}

void CAlerts::initStyle( const std::string& title, const Alert& alert, ImGuiWindow* window )
{
    ImRect const wnd_rect = window->Rect( );

    window->DrawList->PushClipRectFullScreen( );
    // g_blur.applyBlur( wnd_rect, window->DrawList, 16 * GetStyle( ).Scale, 1.f, GetStyle( ).Alpha, false );

    window->DrawList->AddRectFilled( wnd_rect.Min, wnd_rect.Max, Color( CThemes::current.colors.window.background ).modulate( ), 16 * GetStyle( ).Scale );
    CUIElements::drawShadowRect( window->DrawList, wnd_rect.Min, wnd_rect.Max, Color( CThemes::current.colors.window.shadow ).modulate( ), 32, { 0, 0 }, 0, 16 );

    window->DrawList->AddRect( wnd_rect.Min, wnd_rect.Max, Color( CThemes::current.colors.border ).modulate( ), 16 );

    // CUIElements::applyLiquidGlass( [ & ]( ImDrawList* draw ) -> void {
    //     draw->AddRect( wnd_rect.Min, wnd_rect.Max, Color( 32, 32, 32 ).modulate( ), 24 * GetStyle( ).Scale, 0, GetStyle( ).Scale );
    // },
    //                                draw_list, wnd_rect, Color( 32, 32, 32 ).u32( ), Color( 255, 255, 255 ).u32( ) );

    PushFont( everness::ctx->alice->fonts->get( "title" ).ptr );
    window->DrawList->AddText( wnd_rect.Min + ImVec2( static_cast< float >( 15 * GetStyle( ).Scale ), static_cast< float >( 20 * GetStyle( ).Scale ) ), Color( CThemes::current.colors.text ).modulate( ), title.c_str( ) );
    PopFont( );

    window->DrawList->PopClipRect( );
}

void CAlerts::render( )
{
    static CAnimator dim { float( 0.f ) };
    dim.update< float >( !m_alerts.empty( ) ? 1.f : 0.f );
    //
    // if ( dim.get< float >( ) > 0.01f )
    // {
    //     GetForegroundDrawList( )->AddRectFilled( { 0, 0 }, GetIO( ).DisplaySize, ImColor( 0.f, 0.f, 0.f, 120.f / 255.f * dim.get< float >( ) ) );
    // }

    for ( auto it = m_alerts.begin( ); it != m_alerts.end( ); )
    {
        auto& alert = it->second;

        if ( alert.erase && alert.alpha.get< float >( ) < 0.01f )
        {
            if ( alert.task )
                alert.task( alert );
            it = m_alerts.erase( it );
            continue;
        }

        SetNextWindowSize( alert.size );
        SetNextWindowPos( GetIO( ).DisplaySize / 2 - alert.size / 2 );

        alert.alpha.update< float >( alert.erase ? 0.f : 1.f );

        PushStyleVar( ImGuiStyleVar_Alpha, alert.alpha.get< float >( ) );
        PushStyleVar( ImGuiStyleVar_WindowPadding, { 0, 0 } );

        Begin( it->first.c_str( ), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground );
        {
            initStyle( it->first, alert, GetCurrentWindow( ) );

            SetCursorPos( { 10 * GetStyle( ).Scale, 15 * GetStyle( ).Scale * 2 + 16 * GetStyle( ).Scale } ); // just to make sure we're not fucked

            BeginChild( "##area", GetContentRegionAvail( ) - ImVec2( 10 * GetStyle( ).Scale, 10 * GetStyle( ).Scale ), 0 );
            alert.body( alert );
            EndChild( );

            End( );
        }

        PopStyleVar( 2 );

        ++it;
    }
}

bool CAlerts::find( const std::string& title ) const
{
    return m_alerts.contains( title );
}

bool CAlerts::button( const std::string& label, const ImVec2& size, bool primary )
{
    ImGuiWindow* window = GetCurrentWindow( );
    if ( window->SkipItems )
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID( label.c_str( ) );
    const ImVec2 label_size = CalcTextSize( label.c_str( ), NULL, true );

    struct _state
    {
        CAnimator main { 0.0f };

        CAnimator background { ImVec4( 0, 0, 0, 0 ) };
        CAnimator outline { ImVec4( 0, 0, 0, 0 ) };
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ id ];

    ImVec2 const pos = window->DC.CursorPos;
    const ImRect bb( pos, pos + size );
    ItemSize( size, style.FramePadding.y );
    if ( !ItemAdd( bb, id ) )
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );
    if ( pressed )
    {
        state.background.set< ImVec4 >( ImColor( 0, 0, 0 ).Value );
        state.outline.set< ImVec4 >( ImColor( 0, 0, 0 ).Value );
    }

    if ( primary )
    {
        state.background.update< ImVec4 >( Color( CThemes::current.colors.accent ).dim( 0.6f ).u32( ) );
        state.outline.update< ImVec4 >( Color( CThemes::current.colors.accent ).dim( 0.8f ).u32( ) );
    } else
    {
        state.background.update< ImVec4 >( CThemes::current.colors.elem.background );
        state.outline.update< ImVec4 >( CThemes::current.colors.elem.outline );
    }

    state.main.update< float >( held ? 1.f : ( hovered ? 0.8f : 0.f ) );

    GetForegroundDrawList( )->AddRectFilled( bb.Min + ImVec2( 4 * GetStyle( ).Scale, 4 * GetStyle( ).Scale ) * ( 1.0f - state.main.get< float >( ) ), bb.Max - ImVec2( 4 * GetStyle( ).Scale, 4 * GetStyle( ).Scale ) * ( 1.0f - state.main.get< float >( ) ), Color( state.background.get< ImVec4 >( ) ).modulate( ), 64 * GetStyle( ).Scale );
    GetForegroundDrawList( )->AddRectFilled( bb.Min + ImVec2( 4 * GetStyle( ).Scale, 4 * GetStyle( ).Scale ) * ( 1.0f - state.main.get< float >( ) ), bb.Max - ImVec2( 4 * GetStyle( ).Scale, 4 * GetStyle( ).Scale ) * ( 1.0f - state.main.get< float >( ) ), Color( 255, 255, 255, 35 ).modulate( state.main.get< float >( ) * style.Alpha ), 64 * GetStyle( ).Scale );

    CUIElements::applyLiquidGlass( [ & ]( ImDrawList* draw ) -> void {
        draw->AddRect( bb.Min + ImVec2( 4 * GetStyle( ).Scale, 4 * GetStyle( ).Scale ) * ( 1.0f - state.main.get< float >( ) ), bb.Max - ImVec2( 4 * GetStyle( ).Scale, 4 * GetStyle( ).Scale ) * ( 1.0f - state.main.get< float >( ) ), Color( state.outline.get< ImVec4 >( ) ).modulate( ), 64 * GetStyle( ).Scale );
    },
                                   GetForegroundDrawList( ), { bb.Min + ImVec2( 4 * GetStyle( ).Scale, 4 * GetStyle( ).Scale ) * ( 1.0f - state.main.get< float >( ) ), bb.Max - ImVec2( 4 * GetStyle( ).Scale, 4 * GetStyle( ).Scale ) * ( 1.0f - state.main.get< float >( ) ) }, Color( state.outline.get< ImVec4 >( ) ).u32( ), Color( 255, 255, 255 ).u32( ) );

    {
        PushStyleColor( ImGuiCol_Text, primary ? ImVec4( 1, 1, 1, 1 ) : CThemes::current.colors.text );
        const char* text_display_end = FindRenderedTextEnd( label.c_str( ), 0 );
        const int text_len = ( int ) ( text_display_end - label.c_str( ) );
        if ( text_len != 0 )
            RenderTextClippedEx( GetForegroundDrawList( ), bb.Min + style.FramePadding, bb.Max - style.FramePadding, label.c_str( ), text_display_end, &label_size, style.ButtonTextAlign, &bb );
        PopStyleColor( );
    }

    return pressed;
}

void CAlerts::renderContents( ImVec2 pos, const char* text, const char* text_end, float wrap_width )
{
    ImGuiContext& g = *GImGui;

    if ( !text_end )
        text_end = text + strlen( text ); // FIXME-OPT

    if ( text != text_end )
    {
        GetForegroundDrawList( )->AddText( g.Font, g.FontSize, pos, GetColorU32( ImGuiCol_Text ), text, text_end, wrap_width );
    }
}