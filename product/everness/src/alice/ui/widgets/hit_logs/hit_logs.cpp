//
// Created by Kai Tears on 27/01/2026.
//

#include "hit_logs.hpp"

#include "../../../fonts/fonts.hpp"
#include "../../elem/elem.hpp"
//#include "../../../blur/blur.hpp"

#include "core.hpp"

using alice::CHitLogs;

using namespace ImGui;

void CHitLogs::renderColoredText( ImDrawList* draw, ImVec2 pos, const std::string& text, float alpha )
{
    ImColor currentColor = ImColor( 1.f, 1.f, 1.f, alpha );
    float x = pos.x;
    float y = pos.y;

    const char* s = text.c_str( );
    const char* end = s + text.size( );

    while ( s < end )
    {
        const char* tagStart = strstr( s, ( "<color=" ) );
        if ( !tagStart )
        {
            ImVec2 size = CalcTextSize( s );
            draw->AddText( { x, y }, currentColor, s );
            x += size.x;
            break;
        }

        if ( tagStart > s )
        {
            std::string before( s, tagStart - s );
            ImVec2 size = CalcTextSize( before.c_str( ) );
            draw->AddText( { x, y }, currentColor, before.c_str( ) );
            x += size.x;
        }

        const char* colorStart = tagStart + 7;
        const char* colorEnd = strchr( colorStart, '>' );
        if ( !colorEnd )
            break;

        std::string hex( colorStart, colorEnd - colorStart );

        unsigned int r = 255, g = 255, b = 255;
        if ( hex.size( ) == 7 && hex[ 0 ] == '#' )
        {
            r = std::stoi( hex.substr( 1, 2 ), nullptr, 16 );
            g = std::stoi( hex.substr( 3, 2 ), nullptr, 16 );
            b = std::stoi( hex.substr( 5, 2 ), nullptr, 16 );
        }

        currentColor = Color( r, g, b, 255 ).modulate( alpha );

        const char* closeTag = strstr( colorEnd + 1, ( "</color>" ) );
        if ( !closeTag )
            break;

        std::string coloredText( colorEnd + 1, closeTag - ( colorEnd + 1 ) );
        ImVec2 size = CalcTextSize( coloredText.c_str( ) );
        draw->AddText( { x, y }, currentColor, coloredText.c_str( ) );
        x += size.x;

        s = closeTag + 8;
        currentColor = Color( 255, 255, 255, 255 ).modulate( alpha );
    }
}

void CHitLogs::pushLog( const std::string& data )
{
    HitLog log { data };
    log.y.set< float >( GetIO( ).DisplaySize.y / 2 );
    log.timestamp = std::chrono::steady_clock::now( );

    m_logs.push_back( log );
}

std::string CHitLogs::stripColorTags( const std::string& data )
{
    std::string out;
    out.reserve( data.size( ) );

    bool insideTag = false;

    for ( char c : data )
    {
        if ( c == '<' )
        {
            insideTag = true;
            continue;
        }
        if ( c == '>' )
        {
            insideTag = false;
            continue;
        }

        if ( !insideTag )
            out.push_back( c );
    }

    return out;
}

ImVec2 CHitLogs::calcWindowSize( )
{
    float max_width = 0.f;
    float total_height = 0.f;

    for ( auto& log : m_logs )
    {
        std::string const clean = stripColorTags( log.data );
        ImVec2 const sz = CalcTextSize( clean.c_str( ) );

        max_width = std::max( max_width, sz.x );
        total_height += sz.y + 6.f * GetStyle( ).Scale;
    }

    float const pad_x = 20.f * GetStyle( ).Scale;
    float const pad_y = 16.f * GetStyle( ).Scale;

    return {
        max_width + pad_x * 2,
        total_height + pad_y * 2 };
}

void CHitLogs::render( bool allow, bool demo )
{
    static CAnimator alpha { 0.f };
    static CAnimator drag_alpha { 0.f };

    alpha.update< float >( enabled ? 1.f : 0.f );
    auto const opacity = alpha.get< float >( );
    if ( opacity < 0.001f )
        return;

    PushStyleVar( ImGuiStyleVar_Alpha, opacity );

    PushFont( everness::ctx->alice->fonts->get( "monospace" ).ptr );

    // ImVec2 const screen_center = { GetIO( ).DisplaySize.x / 2, GetIO( ).DisplaySize.y / 1.4f };
    auto const now = std::chrono::steady_clock::now( );

    std::erase_if( m_logs, []( const HitLog& n ) {
        return n.alpha.get< float >( ) < 0.01f && n.ended;
    } );

    static bool was_demo = false;

    if ( demo && !was_demo )
    {
        was_demo = true;

        for ( auto& l : m_logs )
            l.ended = true;

        pushLog( ( "Hit <color=#599cff>John Doe</color> in <color=#40ff59>head</color> for <color=#ff6969>100</color> damage" ) );
        pushLog( ( "<color=#ff3b3b>Missed</color> shot due to <color=#ff3b3b>spread</color>" ) );

        for ( auto& l : m_logs )
            l.timestamp = now;
    }

    if ( !demo && was_demo )
    {
        was_demo = false;

        for ( auto& l : m_logs )
            l.ended = true;
    }

    PushStyleColor( ImGuiCol_WindowShadow, { 0, 0, 0, 0 } );
    PushStyleVar( ImGuiStyleVar_WindowPadding, { 0, 0 } );

    SetNextWindowSize( calcWindowSize( ) );

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground;
    if ( !allow )
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;

    Begin( "##hitlogs", 0, flags );

    auto const scale = GetStyle( ).Scale;

    auto const window = GetCurrentWindow( );

    bool const hovering = IsWindowHovered( );
    bool const dragging = IsMouseDragging( 0 );

    drag_alpha.update< float >( allow ? ( ( dragging && hovering ) ? 1.0f : ( hovering ? 0.5f : 0.f ) ) : 0.f );

    renderSelectionOutline( { window->Rect( ).Min + ImVec2( 7, -4 ), window->Rect( ).Max - ImVec2( 7, -5 ) }, drag_alpha.get< float >( ), 12 * scale );

    for ( int it { 0 }; it < m_logs.size( ); it++ )
    {
        auto& log = m_logs[ it ];

        if ( std::chrono::duration< float >( now - log.timestamp ).count( ) > 3.0f && !demo )
            log.ended = true;

        log.alpha.update< float >( log.ended ? 0.f : 1.f );

        auto const data = log.data;
        auto const text_sz = CalcTextSize( stripColorTags( data ).c_str( ) );

        log.win.update< float >( 1.f );
        log.y.update< float >( ( 40 * GetStyle( ).Scale * it ) );
        auto const log_y = ( window->Pos.y * log.win.get< float >( ) + log.y.get< float >( ) );

        ImRect const rect = {
            { ( float ) int( window->Pos.x + window->Size.x / 2 - text_sz.x / 2 - 10 * GetStyle( ).Scale ), ( float ) int( log_y - 0 * GetStyle( ).Scale ) },
            { ( float ) int( window->Pos.x + window->Size.x / 2 + text_sz.x / 2 + 10 * GetStyle( ).Scale ), ( float ) int( log_y + 20 * GetStyle( ).Scale + text_sz.y ) } };

        //g_blur.applyBlur( rect, GetBackgroundDrawList( ), 10 * scale, 1.f, opacity * log.alpha.get< float >( ) );

        GetBackgroundDrawList( )->AddShadowRect( rect.Min + ImVec2( 6 * GetStyle( ).Scale, 6 * GetStyle( ).Scale ), rect.Max - ImVec2( 6 * GetStyle( ).Scale, 6 * GetStyle( ).Scale ), Color( 0, 0, 0 ).modulate( opacity * log.alpha.get< float >( ) ), 32 * GetStyle( ).Scale, { 0, 0 }, 0, 64 * GetStyle( ).Scale );

        GetBackgroundDrawList( )->AddRectFilled( rect.Min, rect.Max, Color( 20, 20, 20, 220 ).modulate( opacity * log.alpha.get< float >( ) ), 10 * GetStyle( ).Scale );
        // GetBackgroundDrawList( )->AddRect( rect.Min, rect.Max, Color( 255, 255, 255, 5 ).modulate( log.alpha.get< float >( ) ), 10 );

        CUIElements::applyLiquidGlass( [ & ]( ImDrawList* draw ) -> void {
            draw->AddRect( rect.Min, rect.Max, Color( 32, 32, 32 ).modulate( opacity * log.alpha.get< float >( ) ), 10 * GetStyle( ).Scale, 0, GetStyle( ).Scale );
        },
                                       GetBackgroundDrawList( ), { rect.Min, rect.Max }, Color( 32, 32, 32 ).u32( ), Color( 255, 255, 255 ).u32( ) );

        renderColoredText( GetBackgroundDrawList( ), { window->Pos.x + window->Size.x / 2 - text_sz.x / 2, log_y + 10 }, data, opacity * log.alpha.get< float >( ) );
    }

    End( );

    PopStyleVar( );
    PopStyleColor( );

    PopFont( );

    PopStyleVar( );
}