//
// Created by Kai Tears on 24/01/2026.
//

#include "elem.hpp"

#include "fonts/fonts.hpp"
#include "fonts/include/font_awesome/font_awesome.hpp"
#include "../popups/popups.hpp"
#include "../themes/themes.hpp"
#include "core.hpp"
#include "ui/alerts/alerts.hpp"

#include <windows.h>
#include <iostream>
#include <unordered_map>
#include <random>
#include <array>

using alice::CUIElements;

using namespace ImGui;

bool CUIElements::unsafe;

float CUIElements::adjHeight( const ElemType& type, const float& input )
{
    return input + ( ( type != ElemType::End && type != ElemType::Single ) ? 1.f : 0.f );
}

void CUIElements::renderBackground( ImDrawList* draw, const ElemType& type, const ImRect& rect, const ImColor& background, const ImColor& outline )
{
    auto style = GetStyle( );

    switch ( type )
    {
    case ElemType::Begin:
    {
        draw->AddRectFilled( rect.Min, rect.Max - ImVec2( 0, 1 ), background, 12 * style.Scale, ImDrawFlags_RoundCornersTop );
        draw->AddLine( { rect.Min.x, rect.Max.y - 1 }, { rect.Max.x, rect.Max.y - 1 }, outline, 1 * style.Scale );
    }
    break;
    case ElemType::End:
    {
        draw->AddRectFilled( rect.Min, rect.Max, background, 12 * style.Scale, ImDrawFlags_RoundCornersBottom );
    }
    break;
    case ElemType::Middle:
    {
        draw->AddRectFilled( rect.Min, rect.Max - ImVec2( 0, 1 ), background );
        draw->AddLine( { rect.Min.x, rect.Max.y - 1 }, { rect.Max.x, rect.Max.y - 1 }, outline, 1 * style.Scale );
    }
    break;
    case ElemType::Single:
    {
        draw->AddRectFilled( rect.Min, rect.Max, background, 12 * style.Scale );
    }
    break;
    default:
        break;
    }
}

ImVec4 CUIElements::gColorClipboard { 1, 1, 1, 1 };

static bool HueBarHorizontalPillFixed( const char* id, float* H, float height )
{
    ImGuiWindow* window = GetCurrentWindow( );
    if ( window->SkipItems )
        return false;

    ImGuiContext& g = *GetCurrentContext( );
    ImGuiIO& io = g.IO;
    ImDrawList* dl = GetWindowDrawList( );

    float w = GetContentRegionAvail( ).x;
    ImVec2 p = GetCursorScreenPos( );
    ImRect bb( p, p + ImVec2( w, height ) );

    ItemSize( bb );
    ImGuiID bid = GetID( id );
    if ( !ItemAdd( bb, bid ) )
        return false;

    bool hovered = false, held = false;
    ButtonBehavior( bb, bid, &hovered, &held );

    if ( *H >= 1.0f )
        *H = 0.999999f;
    if ( *H < 0.0f )
        *H = 0.0f;

    const float r = height * 0.5f;
    const int alpha8 = IM_F32_TO_INT8_SAT( g.Style.Alpha );

    ImRect center( bb.Min + ImVec2( r, 0.f ), bb.Max - ImVec2( r, 0.f ) );
    float cw = center.GetWidth( );
    if ( cw < 1.f )
        cw = 1.f;

    int seg = ( int ) cw;
    if ( seg < 128 )
        seg = 128;
    if ( seg > 512 )
        seg = 512;

    for ( int i = 0; i < seg; i++ )
    {
        float t0 = ( float ) i / ( float ) seg;
        float t1 = ( float ) ( i + 1 ) / ( float ) seg;

        float x0 = center.Min.x + t0 * cw;
        float x1 = ( i == seg - 1 ) ? center.Max.x : ( center.Min.x + t1 * cw );

        x0 = IM_ROUND( x0 );
        x1 = IM_ROUND( x1 );

        float r0, g0, b0, r1, g1, b1;
        ColorConvertHSVtoRGB( t0, 1.f, 1.f, r0, g0, b0 );
        ColorConvertHSVtoRGB( t1, 1.f, 1.f, r1, g1, b1 );

        ImU32 c0 = IM_COL32( ( int ) ( r0 * 255.f ), ( int ) ( g0 * 255.f ), ( int ) ( b0 * 255.f ), alpha8 );
        ImU32 c1 = IM_COL32( ( int ) ( r1 * 255.f ), ( int ) ( g1 * 255.f ), ( int ) ( b1 * 255.f ), alpha8 );

        dl->AddRectFilledMultiColor(
            ImVec2( x0, bb.Min.y ),
            ImVec2( x1, bb.Max.y ),
            c0, c1, c1, c0 );
    }

    float cr, cg, cb;
    ColorConvertHSVtoRGB( 0.f, 1.f, 1.f, cr, cg, cb );
    ImU32 cap = IM_COL32( ( int ) ( cr * 255.f ), ( int ) ( cg * 255.f ), ( int ) ( cb * 255.f ), alpha8 );

    ImVec2 lc( bb.Min.x + r, ( bb.Min.y + bb.Max.y ) * 0.5f );
    ImVec2 rc( bb.Max.x - r, ( bb.Min.y + bb.Max.y ) * 0.5f );
    dl->AddCircleFilled( lc, r, cap, 32 );
    dl->AddCircleFilled( rc, r, cap, 32 );

    // dl->AddRect( bb.Min, bb.Max, IM_COL32( 0, 0, 0, 90 ), r, 0, 1.0f );

    float t = ImClamp( *H, 0.f, 0.999999f );

    static alice::CAnimator anim { 0.f };
    anim.update< float >( t * bb.GetWidth( ) );

    float cx = bb.Min.x + anim.get< float >( );
    cx = ImClamp( cx, bb.Min.x + 2.f, bb.Max.x - 2.f );
    ImVec2 hc( cx, ( bb.Min.y + bb.Max.y ) * 0.5f );

    float hr = r * 1.30f;
    dl->AddShadowCircle( hc, hr, ImColor( 0, 0, 0, alpha8 ), 8, { 0, 0 }, 0 );
    dl->AddCircle( hc, hr, ImColor( 255, 255, 255, alpha8 ), 24, 3 );

    bool changed = false;
    if ( held )
    {
        float nt = ( io.MousePos.x - bb.Min.x ) / bb.GetWidth( );
        nt = ImClamp( nt, 0.f, 1.f );

        if ( nt >= 1.0f )
            nt = 0.999999f;

        if ( nt != *H )
        {
            *H = nt;
            changed = true;
        }
    }

    return changed;
}

std::vector< ImVec4 > CUIElements::gColorPallette { };

void CUIElements::renderColorPicker( ImVec4* color )
{
    auto& style = GetStyle( );
    auto* draw = GetWindowDrawList( );
    float w = GetWindowSize( ).x - 20 * style.Scale + 14 * GetStyle( ).Scale + 4;
    SetNextItemWidth( w );

    struct _state
    {
        CAnimator background { ImVec4( 0, 0, 0, 0 ) };

        CAnimator select { ImVec4( 0, 0, 0, 0 ) };
        CAnimator alpha { 0.f };
    };

    static int selected_color = -1;
    static constexpr auto compare_colors = []( const ImVec4& first, const ImVec4& second ) -> bool {
        return first.x == second.x && first.y == second.y && first.z == second.z && first.w == second.w;
    };

    static std::unordered_map< ImGuiID, _state > __state;

    auto flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoOptions;
    flags &= ~ImGuiColorEditFlags_PickerHueBar;
    flags &= ~ImGuiColorEditFlags_PickerHueWheel;

    ColorPicker4( ( "##picker" + std::to_string( reinterpret_cast< uintptr_t >( color ) ) ).c_str( ),
                  ( float* ) color, flags );

    struct HSVCache
    {
        float h, s, v;
        bool init = false;
    };
    static std::unordered_map< uintptr_t, HSVCache > hsv;

    auto& c = hsv[ ( uintptr_t ) color ];

    auto UpdateHSVFromRGB_NoHueOnGray = [ & ]( const ImVec4& rgb ) {
        float h, s, v;
        ColorConvertRGBtoHSV( rgb.x, rgb.y, rgb.z, h, s, v );

        c.s = s;
        c.v = v;

        if ( s > 0.001f )
            c.h = h;

        if ( c.h >= 1.0f )
            c.h = 0.999999f;
        if ( c.h < 0.0f )
            c.h = 0.0f;
    };

    if ( !c.init )
    {
        UpdateHSVFromRGB_NoHueOnGray( *color );
        c.init = true;
    }

    if ( color->x != 0.f || color->y != 0.f || color->z != 0.f )
        UpdateHSVFromRGB_NoHueOnGray( *color );

    ItemSize( { 0, 10 } );

    bool hue_changed = HueBarHorizontalPillFixed( "##hue_full", &c.h, 8.0f * style.Scale );
    bool hue_active = IsItemActive( );

    if ( hue_changed )
    {
        float r, g, b;
        ColorConvertHSVtoRGB( c.h, c.s, c.v, r, g, b );
        color->x = r;
        color->y = g;
        color->z = b;
    }

    if ( !hue_active )
    {
        if ( color->x != 0.f || color->y != 0.f || color->z != 0.f )
            UpdateHSVFromRGB_NoHueOnGray( *color );
    }

    auto const size = ImVec2( 18, 18 );
    auto const begin = GetCursorScreenPos( ) + ImVec2( 0, 10 );
    auto const avail_width = GetContentRegionAvail( ).x;

    ItemSize( { 0, size.y + 10 } );

    for ( int it { 0 }; it < gColorPallette.size( ); it++ )
    {
        auto const col = gColorPallette[ it ];

        ImRect rect = {
            { begin.x + ( ( size.x + 5 ) * it ), begin.y },
            { begin.x + ( ( size.x + 5 ) * ( it + 1 ) ), begin.y + size.y } };

        ImGuiID cid = GetID( ( "##pallette_at_" + std::to_string( static_cast< float >( it ) + col.w + col.z + col.y + col.x ) ).c_str( ) ) * 24 * ( it + 1 );
        ItemAdd( rect, cid );

        bool hov, held;
        bool pressed = ButtonBehavior( rect, cid, &hov, &held );

        _state& cst = __state[ cid ];
        cst.background.update< ImVec4 >( ( pressed ) ? CThemes::current.colors.elem.backgroundOverlayActive : ( hov ? CThemes::current.colors.elem.backgroundOverlayHovered : Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( 0 ).Value ) );

        draw->AddRectFilled( rect.Min, rect.Max - ImVec2( 5, 0 ), Color( cst.background.get< ImVec4 >( ) ).modulate( ), 4 * style.Scale );

        bool const selected = compare_colors( *color, col ) && selected_color == it;

        cst.select.update< ImVec4 >( selected ? CThemes::current.colors.elem.colorPickerSelect : Color( CThemes::current.colors.elem.colorPickerSelect ).modulate( 0.f ).Value );
        cst.alpha.update< float >( 1.f );

        draw->AddCircleFilled( rect.GetCenter( ), rect.GetHeight( ) / 2, Color( cst.select.get< ImVec4 >( ) ).modulate( ), 24 );

        draw->AddCircleFilled( rect.GetCenter( ), rect.GetHeight( ) / 3, Color( col ).modulate( style.Alpha * cst.alpha.get< float >( ) ), 24 );

        if ( pressed )
        {
            selected_color = it;
            *color = col;
        }
    }

    static std::vector< std::pair< std::string, std::function< void( ) > > > service_btns = {
        std::make_pair( ICON_FA_PLUS, [ & ] {
            //
        } ) };

    for ( int it { 0 }; it < service_btns.size( ); it++ )
    {
        auto const btn = service_btns[ it ];

        ImRect rect = {
            { begin.x + avail_width - ( size.x * ( it + 1 ) ), begin.y },
            { begin.x + avail_width - ( size.x * it ), begin.y + size.y } };

        ImGuiID cid = GetID( btn.first.c_str( ) );
        ItemAdd( rect, cid );

        bool hov, held;
        bool pressed = ButtonBehavior( rect, cid, &hov, &held );

        _state& cst = __state[ cid ];
        cst.background.update< ImVec4 >( ( pressed ) ? CThemes::current.colors.elem.backgroundOverlayActive : ( hov ? CThemes::current.colors.elem.backgroundOverlayHovered : Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( 0 ).Value ) );

        draw->AddRectFilled( rect.Min, rect.Max, Color( cst.background.get< ImVec4 >( ) ).modulate( ), 4 * style.Scale );

        draw->AddText( rect.GetCenter( ) - CalcTextSize( btn.first.c_str( ) ) / 2, Color( CThemes::current.popup.colors.text ).modulate( ), btn.first.c_str( ) );

        if ( pressed )
        {
            // btn.second( );
            if ( gColorPallette.size( ) >= 6 )
                gColorPallette.erase( gColorPallette.begin( ) );

            gColorPallette.push_back( *color );
        }
    }

    PopupEntries::separator( );

    float lum = c.v;
    if ( PopupEntries::sliderFloat( ICON_FA_SUN, "HDR", &lum, 0.1f, 1.f ) )
    {
        c.v = lum;
        float r, g, b;
        ColorConvertHSVtoRGB( c.h, c.s, c.v, r, g, b );
        color->x = r;
        color->y = g;
        color->z = b;
    }

    PopupEntries::sliderFloat( ICON_FA_PAINT_ROLLER, "Alpha", &color->w, 0.f, 1.f );

    if ( PopupEntries::button( ICON_FA_COPY, "Copy" ) )
        gColorClipboard = *color;

    if ( PopupEntries::button( ICON_FA_PASTE, "Paste" ) )
        *color = gColorClipboard;

    static bool search_col = false;
    static bool picking = false;
    static HDC screen_dc = nullptr;

    if ( PopupEntries::button( ICON_FA_EYE_DROPPER, "Pick" ) )
        search_col = true;

    if ( search_col && !picking )
    {
        screen_dc = GetDC( NULL );
        picking = true;
    }

    if ( !search_col && picking )
    {
        ReleaseDC( NULL, screen_dc );
        screen_dc = nullptr;
        picking = false;
    }

    if ( search_col && picking )
    {
        POINT pt { };
        GetCursorPos( &pt );

        COLORREF px = GetPixel( screen_dc, pt.x, pt.y );
        if ( px != CLR_INVALID )
        {
            color->x = GetRValue( px ) / 255.f;
            color->y = GetGValue( px ) / 255.f;
            color->z = GetBValue( px ) / 255.f;
        }

        if ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
            search_col = false;
        if ( GetAsyncKeyState( VK_ESCAPE ) & 0x8000 )
            search_col = false;
    }
}

void CUIElements::pushUnsafe( )
{
    unsafe = true;
}

void CUIElements::popUnsafe( )
{
    unsafe = false;
}

void CUIElements::applyLiquidGlass( const std::function< void( ImDrawList* ) >& base_render, ImDrawList* draw, const ImRect& rect, const ImColor& base_color, const ImColor& highlight_color )
{
    using namespace ImGui;

    base_render( draw );

    {
        const int vert_start_idx = draw->VtxBuffer.Size;
        draw->PushClipRect( rect.Min - ImVec2( 4, 4 ), rect.Min + ImVec2( rect.GetWidth( ) - 4, rect.GetHeight( ) - 4 ), true );
        base_render( draw );
        draw->PopClipRect( );
        const int vert_end_idx = draw->VtxBuffer.Size;

        ShadeVertsLinearColorGradientKeepAlpha( draw, vert_start_idx, vert_end_idx, rect.Min - ImVec2( 20, 20 ), rect.Min + ImVec2( 10, 20 ), highlight_color, base_color );
    }

    {
        const int vert_start_idx = draw->VtxBuffer.Size;
        draw->PushClipRect( rect.Min + ImVec2( 10, 4 ), rect.Max + ImVec2( 8, 8 ), true );
        base_render( draw );
        draw->PopClipRect( );
        const int vert_end_idx = draw->VtxBuffer.Size;

        ShadeVertsLinearColorGradientKeepAlpha( draw, vert_start_idx, vert_end_idx, rect.Max - ImVec2( 10, 20 ), rect.Max + ImVec2( 20, 20 ), base_color, highlight_color );
    }
}

void CUIElements::drawShadowRect( ImDrawList* draw, const ImVec2& obj_min, const ImVec2& obj_max, ImU32 shadow_col, float shadow_thickness, const ImVec2& shadow_offset, ImDrawFlags flags, float obj_rounding )
{
    const int steps = 20;
    const float step_expand = shadow_thickness / steps;
    const float base_alpha = 0.1f;

    for ( int i = 0; i < steps; ++i )
    {
        float t = ( float ) i / steps;
        float offset = step_expand * i;

        float alpha = base_alpha * powf( 1.0f - t, 2.2f );

        draw->AddRectFilled(
            ImVec2( obj_min.x - offset, obj_min.y - offset ),
            ImVec2( obj_max.x + offset, obj_max.y + offset ),
            Color( shadow_col ).modulate( alpha ),
            obj_rounding + std::max( 0.f, offset ) );
    }
}

void CUIElements::text( const std::string& text, bool bottom )
{
    PushFont( everness::ctx->alice->fonts->get( "small" ).ptr );

    auto style = GetStyle( );
    if ( !bottom )
        PushStyleVar( ImGuiStyleVar_ItemSpacing, { 0, 4 * style.Scale } );
    else
        SetCursorPosY( GetCursorPosY( ) - GetStyle( ).ItemSpacing.y + 3 * style.Scale );

    SetCursorPosX( GetCursorPosX( ) + 12 * style.Scale );
    PushStyleColor( ImGuiCol_Text, Color( CThemes::current.colors.elem.overlayText ).modulate( ).Value );
    SetNextItemWidth( GetContentRegionAvail( ).x - 10 * style.Scale );
    TextWrapped( "%s", text.c_str( ) );
    PopStyleColor( );

    if ( !bottom )
        PopStyleVar( );

    PopFont( );
}

bool CUIElements::button( const ElemType& type, const std::string& label )
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
        CAnimator background { ImVec4( 0, 0, 0, 0 ) };
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ id ];

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb( pos, pos + ImVec2( GetContentRegionAvail( ).x, adjHeight( type, kElemHeight ) ) );
    if ( type != ElemType::End && type != ElemType::Single )
        PushStyleVar( ImGuiStyleVar_ItemSpacing, { 0, 0 } );
    ItemSize( total_bb, style.FramePadding.y );
    const bool is_visible = ItemAdd( total_bb, id );
    if ( type != ElemType::End && type != ElemType::Single )
        PopStyleVar( );
    if ( !is_visible )
        return false;

    bool hovered, held;
    bool const pressed = ButtonBehavior( total_bb, id, &hovered, &held );

    if ( pressed )
    {
        state.background.set< ImVec4 >( ImColor( CThemes::current.colors.elem.backgroundClicked ).Value );
    }

    state.background.update< ImVec4 >( ( hovered ) ? ImColor( CThemes::current.colors.elem.backgroundHovered ).Value : ImColor( CThemes::current.colors.elem.background ).Value );
    renderBackground( window->DrawList, type, total_bb, Color( state.background.get< ImVec4 >( ) ).modulate( ), Color( CThemes::current.colors.elem.outline ).modulate( ) );

    const ImVec2 label_pos = ImVec2( total_bb.Min.x + 12 * GetStyle( ).Scale, total_bb.GetCenter( ).y - label_size.y / 2 - adjHeight( type, 0 ) );
    if ( label_size.x > 0.0f )
        RenderText( label_pos, label.c_str( ) );

    auto const arrow_size = CalcTextSize( ICON_FA_ANGLE_RIGHT );
    ImVec2 const arrow_pos = {
        total_bb.Max.x - arrow_size.x - 12,
        total_bb.GetCenter( ).y - arrow_size.y / 2 };

    window->DrawList->AddText( arrow_pos, Color( CThemes::current.colors.text ).modulate( ), ICON_FA_ANGLE_RIGHT );

    return pressed;
}

void CUIElements::color( const ElemType& type, const std::string& label, const std::vector< ImVec4* >& colors )
{
    ImGuiWindow* window = GetCurrentWindow( );
    if ( window->SkipItems )
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID( label.c_str( ) );
    const ImVec2 label_size = CalcTextSize( label.c_str( ), NULL, true );

    struct _state
    {
        CAnimator background { ImVec4( 0, 0, 0, 0 ) };

        CPopup colorPicker { 180, "Color picker", std::nullopt };

        ImVec4* color = nullptr;
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ id ];

    // state.colorPicker.style( ).padding = { 10, 10 };

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb( pos, pos + ImVec2( GetContentRegionAvail( ).x, adjHeight( type, kElemHeight ) ) );
    if ( type != ElemType::End && type != ElemType::Single )
        PushStyleVar( ImGuiStyleVar_ItemSpacing, { 0, 0 } );
    ItemSize( total_bb, style.FramePadding.y );
    const bool is_visible = ItemAdd( total_bb, id );
    if ( type != ElemType::End && type != ElemType::Single )
        PopStyleVar( );
    if ( !is_visible )
        return;

    float const square_sz = 16 * GetStyle( ).Scale;

    ImVec2 const min = { total_bb.Max.x - 12, total_bb.Min.y + ( kElemHeight * GetStyle( ).Scale / 2 - square_sz / 2 ) };

    bool const frame_hovered = ItemHoverable( { total_bb.Min, { min.x - square_sz * ( colors.size( ) ) - ( 6 * style.Scale ) * std::max( static_cast< int >( colors.size( ) ), 0 ), total_bb.Max.y } }, id + 1, 0 );

    state.background.update< ImVec4 >( frame_hovered ? ImColor( CThemes::current.colors.elem.backgroundHovered ).Value : ImColor( CThemes::current.colors.elem.background ).Value );
    renderBackground( window->DrawList, type, total_bb, Color( state.background.get< ImVec4 >( ) ).modulate( ), Color( CThemes::current.colors.elem.outline ).modulate( ) );

    const ImVec2 label_pos = ImVec2( total_bb.Min.x + 12 * GetStyle( ).Scale, total_bb.GetCenter( ).y - label_size.y / 2 - adjHeight( type, 0 ) );
    if ( label_size.x > 0.0f )
        RenderText( label_pos, label.c_str( ) );

    if ( !colors.empty( ) )
    {
        for ( int i = 0; i < colors.size( ); ++i )
        {
            ImVec4* col = colors[ i ];
            if ( !col )
                continue;

            ImRect col_bb(
                ImVec2( min.x - square_sz * ( i + 1 ) - ( 6 * style.Scale ) * i, min.y ),
                ImVec2( min.x - square_sz * i - ( 6 * style.Scale ) * i, min.y + square_sz ) );

            window->DrawList->AddRectFilled( col_bb.Min, col_bb.Max, Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( ), 5 * GetStyle( ).Scale );
            window->DrawList->AddRect( col_bb.Min, col_bb.Max, Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( ), 5 * GetStyle( ).Scale, 0, 1 * style.Scale );
            window->DrawList->AddRectFilled( col_bb.Min, col_bb.Max, Color( *col ).modulate( ), 5 * GetStyle( ).Scale );
            //
            // window->DrawList->AddCircleFilled( col_bb.GetCenter( ), col_bb.GetWidth( ) / 2, Color( 255, 255, 255, 4 ).modulate( ), 64 * GetStyle( ).Scale );
            // window->DrawList->AddCircle( col_bb.GetCenter( ), col_bb.GetWidth( ) / 2, Color( 55, 55, 55 ).modulate( ), 64 * GetStyle( ).Scale, 1 * style.Scale );
            // window->DrawList->AddCircleFilled( col_bb.GetCenter( ), col_bb.GetWidth( ) / 2, Color( *col ).modulate( ), 64 * GetStyle( ).Scale );

            ImGuiID cid = id * 10 * ( i + 1 );
            ItemAdd( col_bb, cid );

            bool hov, held;
            bool pressed = ButtonBehavior( col_bb, cid, &hov, &held );

            _state& cst = __state[ cid ];
            cst.background.update< ImVec4 >( ( pressed || ( state.color == col && state.colorPicker.opened ) ) ? ImColor( CThemes::current.colors.elem.backgroundOverlayActive ).Value : ( hov ? ImColor( CThemes::current.colors.elem.backgroundOverlayHovered ).Value : Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( 0 ).Value ) );

            window->DrawList->AddRectFilled( col_bb.Min - ImVec2( 3 * style.Scale, 3 * style.Scale ), col_bb.Max + ImVec2( 3 * style.Scale, 3 * style.Scale ), Color( cst.background.get< ImVec4 >( ) ).modulate( ), 4 * style.Scale );

            if ( pressed && state.colorPicker.getAnimationValue( ) < 0.01f )
            {
                state.colorPicker.setPosition( { col_bb.Max.x + 5, col_bb.Min.y - 100 } );
                state.colorPicker.opened = true;
                state.color = col;
            }
        }
    }

    PushID( id * 150 );

    state.colorPicker.setTitle( std::to_string( id ) + "##picker__" );
    state.colorPicker.setPadding( { 10, 10 } );

    if ( state.colorPicker.begin( ) )
    {
        renderColorPicker( state.color );

        state.colorPicker.end( );
    }

    PopID( );
}

bool CUIElements::toggle( const ElemType& type, const std::string& label, bool* v, const std::vector< ImVec4* >& colors, const std::optional< std::pair< CPopup*, std::function< void( ) > > >& popup )
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

        CAnimator pill { ImVec4( 0, 0, 0, 0 ) };
        CAnimator toggleBackground { ImVec4( 0, 0, 0, 0 ) };

        CAnimator background { ImVec4( 0, 0, 0, 0 ) };

        CPopup colorPicker { 180, "Color picker", std::nullopt };

        ImVec4* color = nullptr;
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ id ];

    // state.colorPicker.style( ).padding = { 10, 10 };

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb( pos, pos + ImVec2( GetContentRegionAvail( ).x, adjHeight( type, kElemHeight ) ) );
    if ( type != ElemType::End && type != ElemType::Single )
        PushStyleVar( ImGuiStyleVar_ItemSpacing, { 0, 0 } );
    ItemSize( total_bb, style.FramePadding.y );
    const bool is_visible = ItemAdd( total_bb, id );
    if ( type != ElemType::End && type != ElemType::Single )
        PopStyleVar( );
    if ( !is_visible )
        return false;

    ImVec2 const toggle_size = { 28 * GetStyle( ).Scale, ( kElemHeight * GetStyle( ).Scale - 9 * GetStyle( ).Scale * 2 ) };

    ImRect const toggle_rect = {
        { total_bb.Max.x - toggle_size.x - 12 * GetStyle( ).Scale, total_bb.GetCenter( ).y - toggle_size.y / 2 },
        { total_bb.Max.x - 12 * GetStyle( ).Scale, total_bb.GetCenter( ).y + toggle_size.y / 2 } };

    bool hovered, held;
    bool const pressed = ButtonBehavior( toggle_rect, id, &hovered, &held );

    float const square_sz = 16 * GetStyle( ).Scale;

    float const func_padding = popup.has_value( ) ? square_sz : 0;

    ImVec2 const min = { total_bb.Max.x - 46 * GetStyle( ).Scale, total_bb.Min.y + ( kElemHeight * GetStyle( ).Scale / 2 - square_sz / 2 ) };

    bool const frame_hovered = ItemHoverable( { total_bb.Min, { min.x - func_padding - square_sz * ( colors.size( ) ) - ( 6 * style.Scale ) * std::max( static_cast< int >( colors.size( ) ), 0 ), total_bb.Max.y } }, id + 1, 0 );

    if ( pressed )
    {
        state.background.set< ImVec4 >( ImColor( CThemes::current.colors.elem.backgroundClicked ).Value );

        // maybe there's better way, who knows...
        if ( unsafe && !*v )
        {
            if ( everness::ctx->safeMode )
            {
                everness::ctx->alice->alerts->create( "Error", { 260 * GetStyle( ).Scale, 240 * GetStyle( ).Scale }, []( Alert& alert ) {
                    using namespace ImGui;

                    PushStyleColor( ImGuiCol_Text, ImColor( CThemes::current.colors.textDim ).Value );
                    CAlerts::renderContents( GetCursorScreenPos( ) + ImVec2( 5 * GetStyle( ).Scale, 0 ), "This feature is marked as unsafe. Enabling it will disable safe mode. You may get banned for using unsafe features. Are you sure you want to do that?", nullptr, GetWindowSize( ).x - 10 * GetStyle( ).Scale );
                    PopStyleColor( );

                    SetCursorPosY( GetWindowSize( ).y - ( 44 * GetStyle( ).Scale ) * 2 );

                    if ( CAlerts::button( "Disable safe mode", { 240 * GetStyle( ).Scale, 44 * GetStyle( ).Scale }, true ) )
                    {
                        everness::ctx->safeMode = false;
                        alert.close( );
                    }

                    SetCursorPosY( GetWindowSize( ).y - 44 * GetStyle( ).Scale );

                    if ( CAlerts::button( "Cancel", { 240 * GetStyle( ).Scale, 44 * GetStyle( ).Scale }, false ) )
                        alert.close( );
                } );

            } else
                *v = !*v;
        } else
            *v = !*v;
    }

    state.main.update< float >( *v ? 1 : 0 );

    state.toggleBackground.update< ImVec4 >( *v ? ImColor( CThemes::current.colors.accent ).Value : ImColor( CThemes::current.colors.elem.backgroundOverlayHovered ).Value );
    state.pill.update< ImVec4 >( *v ? ImColor( 255, 255, 255 ).Value : ImColor( CThemes::current.colors.elem.backgroundOverlayActive ).Value );

    state.background.update< ImVec4 >( ( frame_hovered || hovered ) ? ImColor( CThemes::current.colors.elem.backgroundHovered ).Value : ImColor( CThemes::current.colors.elem.background ).Value );
    renderBackground( window->DrawList, type, total_bb, Color( state.background.get< ImVec4 >( ) ).modulate( ), Color( CThemes::current.colors.elem.outline ).modulate( ) );

    window->DrawList->AddRectFilled( toggle_rect.Min, toggle_rect.Max, Color( state.toggleBackground.get< ImVec4 >( ) ).dim( 0.65f ).modulate( ), 666 );
    window->DrawList->AddCircleFilled( { toggle_rect.Min.x + std::max( toggle_rect.GetHeight( ) / 2, ( toggle_rect.GetWidth( ) - toggle_rect.GetHeight( ) / 2 ) * state.main.get< float >( ) ), toggle_rect.GetCenter( ).y }, toggle_rect.GetHeight( ) / 2 - 4 * GetStyle( ).Scale, Color( state.pill.get< ImVec4 >( ) ).modulate( ), 666 );

    const ImVec2 unsafe_pos = ImVec2( total_bb.Min.x + 12 * GetStyle( ).Scale, total_bb.GetCenter( ).y - label_size.y / 2 - adjHeight( type, 0 ) );

    auto const unsafe_icon = ICON_FA_TRIANGLE_EXCLAMATION;
    auto const unsafe_size = CalcTextSize( unsafe_icon );

    if ( unsafe )
    {
        PushStyleColor( ImGuiCol_Text, CThemes::current.colors.textUnsafe );

        RenderText( unsafe_pos, unsafe_icon );

        PopStyleColor( );
    }

    const ImVec2 label_pos = ImVec2( unsafe_pos.x + ( unsafe ? ( unsafe_size.x + 8.f * GetStyle( ).Scale ) : 0.f ), total_bb.GetCenter( ).y - label_size.y / 2 - adjHeight( type, 0 ) );
    if ( label_size.x > 0.0f )
        RenderText( label_pos, label.c_str( ) );

    if ( popup.has_value( ) )
    {
        ImRect const col_bb(
            ImVec2( min.x - func_padding - square_sz * ( colors.size( ) ) - ( 6 * style.Scale ) * std::max( static_cast< int >( colors.size( ) ), 0 ), min.y ),
            ImVec2( min.x - square_sz * ( colors.size( ) ) - ( 6 * style.Scale ) * std::max( static_cast< int >( colors.size( ) ), 0 ), min.y + square_sz ) );

        // window->DrawList->AddRectFilled( col_bb.Min, col_bb.Max, Color( 255, 255, 255, 4 ).modulate( ), 5 * GetStyle( ).Scale );
        // window->DrawList->AddRect( col_bb.Min, col_bb.Max, Color( 255, 255, 255, 8 ).modulate( ), 5 * GetStyle( ).Scale, 0, 1 * style.Scale );
        // window->DrawList->AddRectFilled( col_bb.Min, col_bb.Max, Color( *col ).modulate( ), 5 * GetStyle( ).Scale );

        window->DrawList->AddText( col_bb.GetCenter( ) - CalcTextSize( ICON_FA_ANGLE_RIGHT ) / 2 + ImVec2( 1, 0 ), Color( CThemes::current.colors.text ).modulate( ), ICON_FA_ANGLE_RIGHT );

        ImGuiID cid = id * 11;
        ItemAdd( col_bb, cid );

        bool hov, held;
        bool pressed = ButtonBehavior( col_bb, cid, &hov, &held );

        _state& cst = __state[ cid ];
        cst.background.update< ImVec4 >( ( pressed || popup.value( ).first->opened ) ? ImColor( CThemes::current.colors.elem.backgroundOverlayActive ).Value : ( hov ? ImColor( CThemes::current.colors.elem.backgroundOverlayHovered ).Value : Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( 0 ).Value ) );

        window->DrawList->AddRectFilled( col_bb.Min - ImVec2( 3 * style.Scale, 3 * style.Scale ), col_bb.Max + ImVec2( 3 * style.Scale, 3 * style.Scale ), Color( cst.background.get< ImVec4 >( ) ).modulate( ), 4 * style.Scale );

        if ( pressed && !state.colorPicker.opened && state.colorPicker.getAnimationValue( ) < 0.01f && popup.value( ).first->getAnimationValue( ) < 0.01f )
        {
            popup.value( ).first->opened = true;
        }

        popup.value( ).first->setPosition( { col_bb.Min.x - 5, col_bb.Min.y - 30 } );

        popup.value( ).second( );
    }

    if ( !colors.empty( ) )
    {
        for ( int i = 0; i < colors.size( ); ++i )
        {
            ImVec4* col = colors[ i ];
            if ( !col )
                continue;

            ImRect col_bb(
                ImVec2( min.x - square_sz * ( i + 1 ) - ( 6 * style.Scale ) * i, min.y ),
                ImVec2( min.x - square_sz * i - ( 6 * style.Scale ) * i, min.y + square_sz ) );

            window->DrawList->AddRectFilled( col_bb.Min, col_bb.Max, Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( ), 5 * GetStyle( ).Scale );
            window->DrawList->AddRect( col_bb.Min, col_bb.Max, Color( CThemes::current.colors.elem.backgroundOverlayHovered ).modulate( ), 5 * GetStyle( ).Scale, 0, 1 * style.Scale );
            window->DrawList->AddRectFilled( col_bb.Min, col_bb.Max, Color( *col ).modulate( ), 5 * GetStyle( ).Scale );
            //
            // window->DrawList->AddCircleFilled( col_bb.GetCenter( ), col_bb.GetWidth( ) / 2, Color( 255, 255, 255, 4 ).modulate( ), 64 * GetStyle( ).Scale );
            // window->DrawList->AddCircle( col_bb.GetCenter( ), col_bb.GetWidth( ) / 2, Color( 55, 55, 55 ).modulate( ), 64 * GetStyle( ).Scale, 1 * style.Scale );
            // window->DrawList->AddCircleFilled( col_bb.GetCenter( ), col_bb.GetWidth( ) / 2, Color( *col ).modulate( ), 64 * GetStyle( ).Scale );

            ImGuiID cid = id * 10 * ( i + 1 );
            ItemAdd( col_bb, cid );

            bool hov, held;
            bool pressed = ButtonBehavior( col_bb, cid, &hov, &held );

            _state& cst = __state[ cid ];
            cst.background.update< ImVec4 >( ( pressed || ( state.color == col && state.colorPicker.opened ) ) ? ImColor( CThemes::current.colors.elem.backgroundOverlayActive ).Value : ( hov ? ImColor( CThemes::current.colors.elem.backgroundOverlayHovered ).Value : Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( 0 ).Value ) );

            window->DrawList->AddRectFilled( col_bb.Min - ImVec2( 3 * style.Scale, 3 * style.Scale ), col_bb.Max + ImVec2( 3 * style.Scale, 3 * style.Scale ), Color( cst.background.get< ImVec4 >( ) ).modulate( ), 4 * style.Scale );

            if ( pressed && state.colorPicker.getAnimationValue( ) < 0.01f && ( !popup.has_value( ) || ( !popup.value( ).first->opened && popup.value( ).first->getAnimationValue( ) < 0.01f ) ) )
            {
                state.colorPicker.setPosition( { col_bb.Max.x + 5, col_bb.Min.y - 100 } );
                state.colorPicker.opened = true;
                state.color = col;
            }
        }
    }

    PushID( id * 148 );

    state.colorPicker.setTitle( std::to_string( id ) + "##picker_" );
    state.colorPicker.setPadding( { 10, 10 } );

    if ( state.colorPicker.begin( ) )
    {
        renderColorPicker( state.color );

        state.colorPicker.end( );
    }

    PopID( );

    return pressed;
}

bool CUIElements::beginDropdown( const ElemType& type, CPopup* popup, const char* label, const char* preview_value, int items_count )
{
    ImGuiWindow* window = GetCurrentWindow( );

    const ImGuiID id = window->GetID( label );

    struct _state
    {
        CAnimator background { ImVec4( 0, 0, 0, 0 ) };
    };

    const ImVec2 label_size = CalcTextSize( label );
    const ImVec2 preview_size = CalcTextSize( preview_value );
    const ImVec2 pos = window->DC.CursorPos;

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ id ];

    const float width = GetContentRegionAvail( ).x;

    const ImRect dropdown_rect = {
        pos + ImVec2( width / 2, 5 * GetStyle( ).Scale ),
        { pos.x + width - 12 * GetStyle( ).Scale, pos.y + kElemHeight * GetStyle( ).Scale - 5 * GetStyle( ).Scale } };

    const ImRect total_rect = {
        pos,
        { pos.x + width, pos.y + adjHeight( type, kElemHeight * GetStyle( ).Scale ) } };

    if ( type != ElemType::End && type != ElemType::Single )
        PushStyleVar( ImGuiStyleVar_ItemSpacing, { 0, 0 } );
    ItemSize( total_rect, 0 );
    bool const retf = !ItemAdd( total_rect, id );
    if ( type != ElemType::End && type != ElemType::Single )
        PopStyleVar( );
    if ( retf )
        return false;

    bool hovered, held, pressed = ButtonBehavior( total_rect, id, &hovered, &held );

    if ( pressed )
    {
        MarkItemEdited( id );
        if ( !popup->opened && popup->getAnimationValue( ) < 0.01f )
            popup->opened = true;
    }

    state.background.update< ImVec4 >( hovered ? ImColor( CThemes::current.colors.elem.backgroundHovered ).Value : ImColor( CThemes::current.colors.elem.background ).Value );
    renderBackground( window->DrawList, type, total_rect, Color( state.background.get< ImVec4 >( ) ).modulate( ), Color( CThemes::current.colors.elem.outline ).modulate( ) );

    const char* ang_down = "\xef\x84\x87";
    const auto ang_sz = CalcTextSize( ang_down );

    window->DrawList->PushClipRect( dropdown_rect.Min, dropdown_rect.Max, true );

    window->DrawList->AddRectFilled( dropdown_rect.Min, dropdown_rect.Max, Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( ), 6 * GetStyle( ).Scale );
    window->DrawList->AddRect( dropdown_rect.Min, dropdown_rect.Max, Color( CThemes::current.colors.elem.backgroundOverlayHovered ).modulate( ), 6 * GetStyle( ).Scale );

    const int vert_start_idx = window->DrawList->VtxBuffer.Size;
    RenderText( dropdown_rect.Min + ImVec2( 6 * GetStyle( ).Scale, dropdown_rect.GetHeight( ) / 2 - preview_size.y / 2 ), preview_value );
    const int vert_end_idx = window->DrawList->VtxBuffer.Size;
    // ShadeVertsLinearColorGradientKeepAlpha( window->DrawList, vert_start_idx, vert_end_idx, dropdown_rect.Min, { dropdown_rect.Max.x - 30 * GetStyle( ).Scale, dropdown_rect.Max.y }, ImColor( 200, 200, 200 ), ImColor( 32, 32, 32 ) );

    RenderText( { dropdown_rect.Max.x - 8 * GetStyle( ).Scale - ang_sz.x, dropdown_rect.GetCenter( ).y - ang_sz.y / 2 }, ang_down );

    window->DrawList->PopClipRect( );

    const ImVec2 label_pos = ImVec2( total_rect.Min.x + 12 * GetStyle( ).Scale, total_rect.GetCenter( ).y - label_size.y / 2 - adjHeight( type, 0 ) );
    RenderText( label_pos, label );

    auto const full_height = CThemes::current.popup.padding.y * 2 + ( GetFontSize( ) + 14 ) * items_count;

    popup->setWidth( dropdown_rect.GetWidth( ) * 1.6f );
    popup->setMaxHeight( full_height );
    popup->setPosition( { dropdown_rect.GetCenter( ).x - ( dropdown_rect.GetWidth( ) * 1.6f ) / 2, dropdown_rect.GetCenter( ).y - full_height / 2 } );

    return popup->begin( );
}

bool CUIElements::combo( const ElemType& type, const char* label, int* value, const std::vector< std::string >& entries )
{
    ImGuiContext& g = *GImGui;

    struct _state
    {
        std::string randomTitle = []( ) -> const char* {
            static thread_local std::mt19937 rng { std::random_device { }( ) };
            static thread_local std::uniform_int_distribution< int > dist( 'a', 'z' );
            static thread_local std::array< char, 11 > buf { };

            for ( int i = 0; i < 10; ++i )
                buf[ i ] = static_cast< char >( dist( rng ) );
            buf[ 10 ] = '\0';
            return buf.data( );
        }( );
        CPopup popup { 160, "Combo", std::nullopt };
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ GetID( label ) ];

    state.popup.setTitle( state.randomTitle );

    if ( !beginDropdown( type, &state.popup, label, entries[ *value ].c_str( ), entries.size( ) ) )
        return false;

    bool value_changed = false;
    ImGuiListClipper clipper;
    clipper.Begin( entries.size( ) );
    clipper.IncludeItemByIndex( *value );
    while ( clipper.Step( ) )
        for ( int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++ )
        {
            const char* item_text = entries[ i ].c_str( );
            if ( item_text == NULL )
                item_text = "*Unknown item*";

            PushID( i );
            const bool item_selected = ( i == *value );
            bool v = item_selected;
            if ( PopupEntries::toggle( std::nullopt, item_text, &v ) && *value != i )
            {
                value_changed = true;
                *value = i;
            }
            if ( item_selected )
                SetItemDefaultFocus( );
            PopID( );
        }

    state.popup.end( );

    if ( value_changed )
        MarkItemEdited( g.LastItemData.ID );

    return value_changed;
}

bool CUIElements::combo( const ElemType& type, const char* label, int* value, const std::vector< std::pair< ImTextureID, std::string > >& entries )
{
    ImGuiContext& g = *GImGui;

    struct _state
    {
        std::string randomTitle = []( ) -> const char* {
            static thread_local std::mt19937 rng { std::random_device { }( ) };
            static thread_local std::uniform_int_distribution< int > dist( 'a', 'z' );
            static thread_local std::array< char, 11 > buf { };

            for ( int i = 0; i < 10; ++i )
                buf[ i ] = static_cast< char >( dist( rng ) );
            buf[ 10 ] = '\0';
            return buf.data( );
        }( );
        CPopup popup { 160, "Combo", std::nullopt };
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ GetID( label ) ];

    state.popup.setTitle( state.randomTitle );

    if ( !beginDropdown( type, &state.popup, label, entries[ *value ].second.c_str( ), entries.size( ) ) )
        return false;

    bool value_changed = false;
    ImGuiListClipper clipper;
    clipper.Begin( entries.size( ) );
    clipper.IncludeItemByIndex( *value );
    while ( clipper.Step( ) )
        for ( int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++ )
        {
            const char* item_text = entries[ i ].second.c_str( );
            if ( item_text == NULL )
                item_text = "*Unknown item*";

            PushID( i );
            const bool item_selected = ( i == *value );
            bool v = item_selected;
            if ( PopupEntries::toggle( entries[ i ].first, item_text, &v ) && *value != i )
            {
                value_changed = true;
                *value = i;
            }
            if ( item_selected )
                SetItemDefaultFocus( );
            PopID( );
        }

    state.popup.end( );

    if ( value_changed )
        MarkItemEdited( g.LastItemData.ID );

    return value_changed;
}

void CUIElements::multiCombo( const ElemType& type, const std::string& label, const std::vector< std::pair< bool*, std::string > >& entries )
{
    ImGuiContext& g = *GImGui;

    struct _state
    {
        std::string randomTitle = []( ) -> const char* {
            static thread_local std::mt19937 rng { std::random_device { }( ) };
            static thread_local std::uniform_int_distribution< int > dist( 'a', 'z' );
            static thread_local std::array< char, 11 > buf { };

            for ( int i = 0; i < 10; ++i )
                buf[ i ] = static_cast< char >( dist( rng ) );
            buf[ 10 ] = '\0';
            return buf.data( );
        }( );
        CPopup popup { 160, "MultiCombo", std::nullopt };
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ GetID( label.c_str( ) ) ];

    state.popup.setTitle( state.randomTitle );

    auto preview = [ & ]( ) -> std::string {
        std::string out;
        int shown = 0;

        for ( auto& [ flag, name ] : entries )
        {
            if ( !*flag )
                continue;

            if ( !out.empty( ) )
                out += ", ";

            out += name;

            if ( ++shown >= 2 )
                break;
        }

        if ( shown == 0 )
            out = "None";
        else if ( shown < ( int ) entries.size( ) )
            out += ", ...";

        return out;
    };

    if ( !beginDropdown( type, &state.popup, label.c_str( ), preview( ).c_str( ), entries.size( ) ) )
        return;

    for ( auto& pair : entries )
        PopupEntries::toggle( std::nullopt, pair.second, pair.first );

    state.popup.end( );
}

void CUIElements::config( const std::string& name, const int& id, const ImVec2& size_arg, bool selected, const std::function< void( ) > delete_fn, const std::function< void( ) > load_fn, const std::function< void( ) > update_fn )
{
    ImGuiWindow* window = GetCurrentWindow( );
    if ( window->SkipItems )
        return;

    struct _state
    {
        CAnimator text { ImVec4( 0, 0, 0, 0 ) };

        CAnimator main { float( 0.f ) };
        CAnimator pos { ImVec2( 0.f, 0.f ) };
    };

    static std::unordered_map< ImGuiID, _state > __state;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID label_id = window->GetID( name.c_str( ) );
    const ImVec2 label_size = CalcTextSize( name.c_str( ), NULL, true );

    _state& state = __state[ label_id ];
    state.main.update< float >( 1.f );

    auto const cursor = GetCursorPos( );
    state.pos.update< ImVec2 >( cursor );

    ImVec2 scroll = window->Scroll; // (x,y)
    ImVec2 pos = window->Pos + state.pos.get< ImVec2 >( ) - scroll;

    ImVec2 size = CalcItemSize( size_arg, GetContentRegionAvail( ).x, label_size.y + style.FramePadding.y * 6.0f );

    const ImRect bb( pos, pos + size );
    ItemSize( size, style.FramePadding.y );
    if ( !ItemAdd( bb, label_id ) )
        return;

    PushStyleVar( ImGuiStyleVar_Alpha, style.Alpha * state.main.get< float >( ) );

    GetWindowDrawList( )->AddRectFilled( bb.Min, bb.Max, Color( CThemes::current.colors.elem.background ).modulate( ), 10 * GetStyle( ).Scale );

    ImVec2 const center_pos = { bb.Min.x + 10 * GetStyle( ).Scale, bb.GetCenter( ).y };

    auto const btn_size = 20 * GetStyle( ).Scale;
    ImVec2 const start = { bb.Max.x - 10 * GetStyle( ).Scale - ( btn_size * 3 ) - ( 10 * GetStyle( ).Scale * 2 ), bb.GetCenter( ).y - btn_size / 2 };

    state.text.update< ImVec4 >( selected
                                     ? CThemes::current.colors.accent
                                     : CThemes::current.colors.text );

    GetWindowDrawList( )->AddText( center_pos - ImVec2( 0, 2 * GetStyle( ).Scale + label_size.y ), Color( state.text.get< ImVec4 >( ) ).modulate( ), name.c_str( ) );
    GetWindowDrawList( )->AddText( center_pos + ImVec2( 0, 2 * GetStyle( ).Scale ), Color( CThemes::current.colors.textDim ).modulate( ), std::to_string( id ).c_str( ) );

    std::vector< std::pair< std::string, std::function< void( ) > > > inner_buttons = {
        { ICON_FA_DOWNLOAD, load_fn },
        { ICON_FA_FLOPPY_DISK, update_fn },
        { ICON_FA_TRASH_CAN, delete_fn } };

    for ( int it { 0 }; it < inner_buttons.size( ); it++ )
    {
        auto& btn = inner_buttons[ it ].first;

        ImRect const rect = { start + ImVec2( btn_size * it + ( 10 * GetStyle( ).Scale * it ), 0 ), start + ImVec2( btn_size * ( it + 1 ) + ( 10 * GetStyle( ).Scale * it ), btn_size ) };

        auto const inner_id = GetID( ( std::to_string( it ) + inner_buttons[ it ].first + name + std::to_string( id ) ).c_str( ) );

        ItemAdd( rect, inner_id );

        bool hovered, held;
        bool pressed = ButtonBehavior( rect, inner_id, &hovered, &held, 0 );

        _state& state = __state[ inner_id ];
        state.text.update< ImVec4 >( hovered
                                         ? ImColor( CThemes::current.colors.textHover ).Value
                                         : ImColor( CThemes::current.colors.textDim ).Value );

        GetWindowDrawList( )->AddText( rect.GetCenter( ) - CalcTextSize( btn.c_str( ) ) / 2, Color( state.text.get< ImVec4 >( ) ).modulate( ), btn.c_str( ) );

        if ( pressed )
        {
            state.text.set< ImVec4 >( ImColor( CThemes::current.colors.accent ).Value );
            inner_buttons[ it ].second( );
        }
    }

    PopStyleVar( );
}
