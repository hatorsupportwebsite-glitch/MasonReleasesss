//
// Created by Kai Tears on 25/01/2026.
//

#include "popups.hpp"

// #include "../../blur/blur.hpp"
#include "../../fonts/include/font_awesome/font_awesome.hpp"
#include "../elem/elem.hpp"
#include "../themes/themes.hpp"

#include <iostream>
#include <unordered_map>
#include <array>
#include <random>
#include <variant>

using alice::CPopup;
using alice::PopupEntries;

using namespace ImGui;

CPopup::~CPopup( ) = default;

void CPopup::applyBlur( ImGuiWindow* window )
{
    if ( !window )
        return;

    if ( m_popupBlur.get< float >( ) < 0.01f || m_popupOpacity.get< float >( ) < 0.01f )
        return;

    // g_blur.applyBlur( window->Rect( ), window->DrawList, window->WindowRounding, m_popupBlur.get< float >( ) * 4, m_popupOpacity.get< float >( ), false );
}

CPopup::CPopup( float width, const std::string& title, const std::optional< ImVec2 >& force_pos )
{
    m_width = width;
    m_title = title;
    m_forcePos = force_pos;
}

bool CPopup::begin( bool handle )
{
    m_popupBlur.update< float >( opened ? 0.f : 1.f, 0.25f );
    m_popupOpacity.update< float >( ( opened || m_popupBlur.get< float >( ) < 0.38f ) ? 1.f : 0.f, 0.05f );

    if ( m_popupOpacity.get< float >( ) < 0.005f )
    {
        m_setFocus = true;
        m_mousePos = GetMousePos( );
        return false;
    }

    // reverse ts
    if ( CThemes::current.popup.animatePadding )
        SetNextWindowPos( m_forcePos.value_or( m_mousePos - ImVec2( 10, 10 ) * ( 1.0f - m_popupOpacity.get< float >( ) ) ), ImGuiCond_Always );
    else
        SetNextWindowPos( m_forcePos.value_or( m_mousePos ), ImGuiCond_Always );

    SetNextWindowSize( { m_width, m_maxHeight.value_or( 0.f ) } );

    auto const padding = m_forcePadding.value_or( CThemes::current.popup.padding );

    if ( CThemes::current.popup.animatePadding )
        PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2 { padding.x + ( 10 * ( 1.f - m_popupOpacity.get< float >( ) ) ), padding.y + ( 10 * ( 1.f - m_popupOpacity.get< float >( ) ) ) } );
    else
        PushStyleVar( ImGuiStyleVar_WindowPadding, padding );

    PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 0 } );
    PushStyleVar( ImGuiStyleVar_Alpha, GetStyle( ).Alpha * m_popupOpacity.get< float >( ) );
    PushStyleVar( ImGuiStyleVar_WindowRounding, CThemes::current.popup.rounding );

    PushStyleColor( ImGuiCol_WindowBg, { 0, 0, 0, 0 } );

    auto flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if ( !opened )
        flags |= ImGuiWindowFlags_NoInputs;

    Begin( m_title.c_str( ), nullptr, flags );
    auto* window = GetCurrentWindow( );

    if ( m_setFocus )
    {
        BringWindowToDisplayFront( GetCurrentWindow( ) );
        m_setFocus = false;
    }

    window->DrawList->PushClipRectFullScreen( );
    ImVec2 shadow_offset = ImVec2( ImCos( IM_PI * 0.25f ), ImSin( IM_PI * 0.25f ) ) * 0;
    CUIElements::drawShadowRect( window->DrawList, window->Pos, window->Pos + window->Size, Color( CThemes::current.popup.colors.shadow ).modulate( ), CThemes::current.popup.shadow.size, shadow_offset, ImDrawFlags_ShadowCutOutShapeBackground, CThemes::current.popup.rounding );
    window->DrawList->AddRectFilled( window->Pos, window->Pos + window->Size, Color( CThemes::current.popup.colors.background ).modulate( ), CThemes::current.popup.rounding );
    window->DrawList->AddRect( window->Pos, window->Pos + window->Size, Color( CThemes::current.popup.colors.outline ).modulate( ), CThemes::current.popup.rounding );
    window->DrawList->PopClipRect( );

    if ( !IsWindowHovered( ) && opened && GetIO( ).MouseClicked[ 0 ] && !handle )
        opened = false;

    m_window = window;

    if ( m_window->SkipItems )
    {
        end( );
        return false;
    }

    return true;
}

void CPopup::end( )
{
    PopStyleVar( 4 );
    PopStyleColor( );

    applyBlur( m_window );
    End( );
}

float CPopup::getAnimationValue( )
{
    return m_popupOpacity.get< float >( );
}

void CPopup::setPadding( const ImVec2& padding )
{
    m_forcePadding = padding;
}

void CPopup::setWidth( const float& width )
{
    m_width = width;
}

void CPopup::setMaxHeight( const float& height )
{
    m_maxHeight = height;
}

void CPopup::setPosition( const ImVec2& pos )
{
    m_forcePos = pos;
}

void CPopup::setTitle( const std::string& title )
{
    m_title = title;
}

bool CPopup::allowOpen( )
{
    return !opened && m_popupOpacity.get< float >( ) < 0.01f;
}

std::optional< float > PopupEntries::itemHeightOverride { std::nullopt };
std::optional< ImColor > PopupEntries::textColorOverride { std::nullopt };
std::optional< CPopup* > PopupEntries::popupEnablerCallback { std::nullopt };

void PopupEntries::separator( )
{
    ImGuiWindow* window = GetCurrentWindow( );
    if ( window->SkipItems )
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = { GetContentRegionAvail( ).x, itemHeightOverride.value_or( 21 ) };

    window->DrawList->AddLine( { pos.x + 2, pos.y + 10 }, { pos.x + size.x - 2, pos.y + 10 }, Color( CThemes::current.popup.colors.outline ).modulate( ), 1.0f );

    ItemSize( size );
}

bool PopupEntries::button( const std::optional< std::string >& icon, const std::string& title )
{
    ImColor const text_color = textColorOverride.value_or( CThemes::current.popup.colors.text );
    textColorOverride = std::nullopt;

    ImGuiWindow* window = GetCurrentWindow( );
    if ( window->SkipItems )
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID( title.c_str( ) );
    const ImVec2 label_size = CalcTextSize( title.c_str( ), NULL, true );

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = { GetContentRegionAvail( ).x, itemHeightOverride.value_or( GetFontSize( ) + 14 ) };

    const ImRect bb( pos, pos + size );
    ItemSize( size, style.FramePadding.y );
    if ( !ItemAdd( bb, id ) )
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    if ( hovered && g.IO.MouseClicked[ 1 ] && popupEnablerCallback.has_value( ) )
    {
        popupEnablerCallback.value( )->opened = true;
    }

    struct _state
    {
        CAnimator background { ImVec4( 0, 0, 0, 0 ) };
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ id ];

    if ( pressed )
    {
        state.background.set< ImVec4 >( CThemes::current.popup.colors.overlayActive );
    }

    state.background.update< ImVec4 >( hovered ? CThemes::current.popup.colors.overlayHovered : CThemes::current.popup.colors.overlay );

    window->DrawList->AddRectFilled( bb.Min, bb.Max, Color( state.background.get< ImVec4 >( ) ).modulate( ), CThemes::current.popup.rounding );

    if ( icon.has_value( ) )
        window->DrawList->AddText( { bb.Min.x + ( CThemes::current.popup.padding.x + 2 ) + GetFontSize( ) / 2 - ( CalcTextSize( icon.value( ).c_str( ) ).x - 1 ) / 2, bb.GetCenter( ).y - CalcTextSize( icon.value( ).c_str( ) ).y / 2 }, Color( text_color ).modulate( ), icon.value( ).c_str( ) );

    window->DrawList->AddText( { bb.Min.x + CThemes::current.popup.padding.x + 2 + ( ( CThemes::current.popup.spaceIcons || icon.has_value( ) ) ? ( CThemes::current.popup.padding.x + 12 ) : 0 ), bb.GetCenter( ).y - label_size.y / 2 }, Color( text_color ).modulate( ), title.c_str( ) );

    itemHeightOverride = std::nullopt;
    popupEnablerCallback = std::nullopt;

    return pressed;
}

bool PopupEntries::toggle( const std::optional< std::variant< std::string, ImTextureID > >& icon, const std::string& title, bool* v )
{
    ImColor const text_color = textColorOverride.value_or( CThemes::current.popup.colors.text );
    textColorOverride = std::nullopt;

    ImGuiWindow* window = GetCurrentWindow( );
    if ( window->SkipItems )
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID( title.c_str( ) );
    const ImVec2 label_size = CalcTextSize( title.c_str( ), NULL, true );

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = { GetContentRegionAvail( ).x, GetFontSize( ) + 14 };

    const ImRect bb( pos, pos + size );
    ItemSize( size, style.FramePadding.y );
    if ( !ItemAdd( bb, id ) )
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    if ( hovered && g.IO.MouseClicked[ 1 ] && popupEnablerCallback.has_value( ) )
    {
        popupEnablerCallback.value( )->opened = true;
    }

    struct _state
    {
        CAnimator background { ImVec4( 0, 0, 0, 0 ) };
        CAnimator checkmark { ImVec4( 0, 0, 0, 0 ) };
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ id ];

    if ( pressed )
    {
        state.background.set< ImVec4 >( CThemes::current.popup.colors.overlayActive );
        *v = !( *v );
    }

    state.checkmark.update< ImVec4 >( *v ? Color( CThemes::current.colors.accent ).modulate( ).Value : Color( CThemes::current.colors.accent ).modulate( 0.f ).Value );
    state.background.update< ImVec4 >( hovered ? CThemes::current.popup.colors.overlayHovered : ImVec4 { CThemes::current.popup.colors.overlay } );

    window->DrawList->AddRectFilled( bb.Min, bb.Max, Color( state.background.get< ImVec4 >( ) ).modulate( ), CThemes::current.popup.rounding );

    const float square_sz = 12.0f;
    const float pad = ImMax( 1.0f, IM_TRUNC( square_sz / 6.0f ) );

    RenderCheckMark(
        window->DrawList,
        { bb.Max.x - square_sz - CThemes::current.popup.padding.x, bb.GetCenter( ).y - square_sz * 0.5f + 2 },
        Color( state.checkmark.get< ImVec4 >( ) ).modulate( style.Alpha ),
        square_sz - pad * 2.0f );

    if ( icon.has_value( ) )
    {
        std::visit( [ & ]( auto&& value ) {
            using T = std::decay_t< decltype( value ) >;

            if constexpr ( std::is_same_v< T, std::string > )
            {
                window->DrawList->AddText( { bb.Min.x + CThemes::current.popup.padding.x + 2 + GetFontSize( ) / 2 - ( CalcTextSize( value.c_str( ) ).x - 1 ) / 2, bb.GetCenter( ).y - CalcTextSize( value.c_str( ) ).y / 2 }, Color( text_color ).modulate( ), value.c_str( ) );
            } else if constexpr ( std::is_same_v< T, ImTextureID > )
            {
                const float icon_size = 20.0f;

                ImVec2 icon_pos = {
                    bb.Min.x + CThemes::current.popup.padding.x + 2.0f,
                    bb.GetCenter( ).y - icon_size / 2.0f };

                window->DrawList->AddImage(
                    value,
                    icon_pos,
                    ImVec2( icon_pos.x + icon_size, icon_pos.y + icon_size ), { 0, 0 }, { 1, 1 }, Color( 255, 255, 255 ).modulate( ) );
            }
        },
                    *icon );
    }

    window->DrawList->AddText( { bb.Min.x + CThemes::current.popup.padding.x + 2 + ( ( CThemes::current.popup.spaceIcons || icon.has_value( ) ) ? ( CThemes::current.popup.padding.x + [ & ]( ) -> float {
                                     if ( icon.has_value( ) )
                                     {
                                         float result { 0.f };

                                         std::visit( [ & ]( auto&& value ) {
                                             using T = std::decay_t< decltype( value ) >;
                                             if constexpr ( std::is_same_v< T, std::string > )
                                             {
                                                 result += 12.f;
                                             } else if constexpr ( std::is_same_v< T, ImTextureID > )
                                             {
                                                 result += 18.f;
                                             }
                                         },
                                                     *icon );

                                         return result;
                                     }

                                     return 0.f;
                                 }( ) )
                                                                                                                                                : 0 ),
                                 bb.GetCenter( ).y - label_size.y / 2 },
                               Color( text_color ).modulate( ), title.c_str( ) );

    textColorOverride = std::nullopt;
    itemHeightOverride = std::nullopt;
    popupEnablerCallback = std::nullopt;

    return pressed;
}

bool PopupEntries::sliderScalar( const std::optional< std::string >& icon, const std::string& title, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags )
{
    ImColor const text_color = textColorOverride.value_or( CThemes::current.popup.colors.text );
    textColorOverride = std::nullopt;

    ImGuiWindow* window = GetCurrentWindow( );
    if ( window->SkipItems )
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID( title.c_str( ) );

    const ImVec2 label_size = CalcTextSize( title.c_str( ), NULL, true );

    auto const height = GetFontSize( ) + 14;
    const ImRect frame_bb( window->DC.CursorPos + ImVec2( GetContentRegionAvail( ).x / 2, 11 ), window->DC.CursorPos + ImVec2 { GetContentRegionAvail( ).x - 10, height - 10 } );
    const ImRect total_bb( window->DC.CursorPos, window->DC.CursorPos + ImVec2 { GetContentRegionAvail( ).x, height } );

    const bool temp_input_allowed = ( flags & ImGuiSliderFlags_NoInput ) == 0;
    ItemSize( total_bb, style.FramePadding.y );
    if ( !ItemAdd( total_bb, id, &frame_bb ) )
        return false;

    // Default format string when passing NULL
    if ( format == NULL )
        format = DataTypeGetInfo( data_type )->PrintFmt;

    const bool hovered = ItemHoverable( total_bb, id, g.LastItemData.ItemFlags );

    bool temp_input_is_active = temp_input_allowed && TempInputIsActive( id );
    if ( !temp_input_is_active )
    {
        // Tabbing or CTRL-clicking on Slider turns it into an input box
        const bool clicked = hovered && IsMouseClicked( 0, ImGuiInputFlags_None, id );
        const bool make_active = ( clicked || g.NavActivateId == id );
        if ( make_active && clicked )
            SetKeyOwner( ImGuiKey_MouseLeft, id );
        if ( make_active && temp_input_allowed )
            if ( ( clicked && g.IO.KeyCtrl ) || ( g.NavActivateId == id && ( g.NavActivateFlags & ImGuiActivateFlags_PreferInput ) ) )
                temp_input_is_active = true;

        // Store initial value (not used by main lib but available as a convenience but some mods e.g. to revert)
        if ( make_active )
            memcpy( &g.ActiveIdValueOnActivation, p_data, DataTypeGetInfo( data_type )->Size );

        if ( make_active && !temp_input_is_active )
        {
            SetActiveID( id, window );
            SetFocusID( id, window );
            FocusWindow( window );
            g.ActiveIdUsingNavDirMask |= ( 1 << ImGuiDir_Left ) | ( 1 << ImGuiDir_Right );
        }
    }

    struct _state
    {
        CAnimator background { ImVec4( 0, 0, 0, 0 ) };
        CAnimator grab { 0.0f };
        CAnimator value { 0.f };
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ id ];

    // Slider behavior
    ImRect grab_bb;
    const bool value_changed = SliderBehavior( frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb );
    if ( value_changed )
        MarkItemEdited( id );

    state.grab.update< float >( grab_bb.Max.x - frame_bb.Min.x );

    window->DrawList->AddRectFilled( frame_bb.Min, frame_bb.Max, Color( CThemes::current.popup.colors.sliderBackground ).modulate( ), 6 );
    window->DrawList->AddRectFilled( frame_bb.Min, ImVec2( frame_bb.Min.x + state.grab.get< float >( ), frame_bb.Max.y ), Color( CThemes::current.colors.accent ).modulate( 0.95f * style.Alpha ), 6 );

    // Render grab

    if ( hovered && g.IO.MouseClicked[ 1 ] && popupEnablerCallback.has_value( ) )
    {
        popupEnablerCallback.value( )->opened = true;
    }

    state.background.update< ImVec4 >( hovered ? CThemes::current.popup.colors.overlayHovered : CThemes::current.popup.colors.overlay );

    window->DrawList->AddRectFilled( total_bb.Min, total_bb.Max, Color( state.background.get< ImVec4 >( ) ).modulate( ), CThemes::current.popup.rounding );

    if ( icon.has_value( ) )
        window->DrawList->AddText( { total_bb.Min.x + CThemes::current.popup.padding.x + 2 + GetFontSize( ) / 2 - ( CalcTextSize( icon.value( ).c_str( ) ).x - 1 ) / 2, total_bb.GetCenter( ).y - CalcTextSize( icon.value( ).c_str( ) ).y / 2 }, Color( text_color ).modulate( ), icon.value( ).c_str( ) );

    window->DrawList->AddText( { total_bb.Min.x + CThemes::current.popup.padding.x + 2 + ( ( CThemes::current.popup.spaceIcons || icon.has_value( ) ) ? ( CThemes::current.popup.padding.x + 12 ) : 0 ), total_bb.GetCenter( ).y - label_size.y / 2 }, Color( text_color ).modulate( ), title.c_str( ) );

    state.value.update< float >( value_changed || IsItemActive( ) ? 1.f : 0.f );

    if ( state.value.get< float >( ) > 0.001f )
    {
        window->DrawList->PushClipRectFullScreen( );

        char value_buf[ 64 ];
        const char* value_buf_end = value_buf + DataTypeFormatString( value_buf, IM_ARRAYSIZE( value_buf ), data_type, p_data, format );

        auto const value_size = CalcTextSize( value_buf ) - ImVec2( 1, 0 );

        ImVec2 const value_pos = {
            frame_bb.Min.x + state.grab.get< float >( ) - std::max( value_size.x, 10.f ) - 2,
            frame_bb.Min.y - value_size.y - 6 };

        ImRect const value_rect = {
            value_pos - ImVec2( 3, 3 ),
            value_pos + ImVec2( std::max( value_size.x, 10.f ) + 3, value_size.y + 3 ) };

        window->DrawList->AddRectFilled( value_rect.Min, value_rect.Max, Color( CThemes::current.colors.window.background ).modulate( style.Alpha * state.value.get< float >( ) ), 5 );
        // g_blur.applyBlur( value_rect, window->DrawList, 5, 1.f, state.value.get< float >( ) );

        window->DrawList->AddText( value_rect.GetCenter( ) - value_size / 2, Color( text_color ).modulate( style.Alpha * state.value.get< float >( ) ), value_buf );

        window->DrawList->PopClipRect( );
    }

    textColorOverride = std::nullopt;
    itemHeightOverride = std::nullopt;
    popupEnablerCallback = std::nullopt;

    return value_changed;
}

bool PopupEntries::sliderFloat( const std::optional< std::string >& icon, const std::string& title, float* v, float min, float max )
{
    return sliderScalar( icon, title, ImGuiDataType_Float, v, &min, &max, nullptr, 0 );
}

bool PopupEntries::sliderInt( const std::optional< std::string >& icon, const std::string& title, int* v, int min, int max )
{
    return sliderScalar( icon, title, ImGuiDataType_S32, v, &min, &max, nullptr, 0 );
}

bool PopupEntries::bindType( const std::optional< std::string >& icon, const std::string& title, BindType* bind )
{
    ImColor const text_color = textColorOverride.value_or( CThemes::current.popup.colors.text );
    textColorOverride = std::nullopt;

    ImGuiWindow* window = GetCurrentWindow( );
    if ( window->SkipItems )
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID( title.c_str( ) );
    const ImVec2 label_size = CalcTextSize( title.c_str( ), NULL, true );

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = { GetContentRegionAvail( ).x, itemHeightOverride.value_or( GetFontSize( ) + 14 ) };

    const ImRect bb( pos, pos + size );
    ItemSize( size, style.FramePadding.y );
    if ( !ItemAdd( bb, id ) )
        return false;

    static std::vector< std::pair< std::string, BindType > > types = {
        { ICON_FA_FINGERPRINT, BindType::Hold },
        { ICON_FA_KEY, BindType::Toggle } };

    ImVec2 const btn_size = { size.y, size.y };

    auto const full_width = btn_size.x * types.size( ) + 4;

    ImVec2 const min = {
        bb.Max.x - full_width,
        bb.Min.y };

    bool hovered, held;
    ButtonBehavior( { bb.Min, bb.Max - ImVec2( full_width, 0 ) }, id, &hovered, &held );

    struct _state
    {
        CAnimator background { ImVec4( 0, 0, 0, 0 ) };
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ id ];

    for ( int it { 0 }; it < types.size( ); it++ )
    {
        const auto& type = types[ it ];

        auto const bind_type = type.second;
        auto const bind_icon = type.first;

        ImRect bb(
            ImVec2( min.x + btn_size.x * it, min.y ),
            ImVec2( min.x + btn_size.x * ( it + 1 ), min.y + btn_size.y ) );

        ImGuiID cid = id * 10 * ( it + 1 );
        ItemAdd( bb, cid );

        bool hov, held;
        bool pressed = ButtonBehavior( bb, cid, &hov, &held );

        _state& cst = __state[ cid ];
        cst.background.update< ImVec4 >( ( pressed || ( *bind == bind_type ) ) ? CThemes::current.colors.elem.backgroundOverlayActive : ( hov ? CThemes::current.colors.elem.backgroundOverlayHovered : Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( 0 ).Value ) );

        window->DrawList->AddRectFilled( bb.Min + ImVec2( 3 * style.Scale, 3 * style.Scale ), bb.Max - ImVec2( 3 * style.Scale, 3 * style.Scale ), Color( cst.background.get< ImVec4 >( ) ).modulate( ), 4 * style.Scale );

        window->DrawList->AddText( bb.GetCenter( ) - CalcTextSize( bind_icon.c_str( ) ) / 2, Color( text_color ).modulate( ), bind_icon.c_str( ) );

        if ( pressed )
        {
            *bind = bind_type;
        }
    }

    state.background.update< ImVec4 >( hovered ? CThemes::current.popup.colors.overlayHovered : CThemes::current.popup.colors.overlayActive );
    //
    // window->DrawList->AddRectFilled( bb.Min, bb.Max, ImColor( state.background.get< ImVec4 >( ) ), CThemes::current.popup.rounding );

    if ( icon.has_value( ) )
        window->DrawList->AddText( { bb.Min.x + CThemes::current.popup.padding.x + 2 + GetFontSize( ) / 2 - ( CalcTextSize( icon.value( ).c_str( ) ).x - 1 ) / 2, bb.GetCenter( ).y - CalcTextSize( icon.value( ).c_str( ) ).y / 2 }, Color( text_color ).modulate( ), icon.value( ).c_str( ) );

    window->DrawList->AddText( { bb.Min.x + CThemes::current.popup.padding.x + 2 + ( ( CThemes::current.popup.spaceIcons || icon.has_value( ) ) ? ( CThemes::current.popup.padding.x + 12 ) : 0 ), bb.GetCenter( ).y - label_size.y / 2 }, Color( text_color ).modulate( ), title.c_str( ) );

    itemHeightOverride = std::nullopt;
    textColorOverride = std::nullopt;
    popupEnablerCallback = std::nullopt;

    return true;
}

bool PopupEntries::bindListener( const std::optional< std::string >& icon, const std::string& title, ImGuiKey* bind, bool* listening )
{
    ImColor const text_color = textColorOverride.value_or( CThemes::current.popup.colors.text );
    textColorOverride = std::nullopt;

    ImGuiWindow* window = GetCurrentWindow( );
    if ( window->SkipItems )
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID( title.c_str( ) );
    const ImVec2 label_size = CalcTextSize( title.c_str( ), NULL, true );

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = { GetContentRegionAvail( ).x, itemHeightOverride.value_or( GetFontSize( ) + 14 ) };

    const ImRect bb( pos, pos + size );
    ItemSize( size, style.FramePadding.y );
    if ( !ItemAdd( bb, id ) )
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held );

    struct _state
    {
        CAnimator background { ImVec4( 0, 0, 0, 0 ) };
    };

    static std::unordered_map< ImGuiID, _state > __state;
    _state& state = __state[ id ];

    if ( pressed )
    {
        state.background.set< ImVec4 >( CThemes::current.popup.colors.overlayActive );
        *listening = true;
    }

    state.background.update< ImVec4 >( hovered ? CThemes::current.popup.colors.overlayHovered : CThemes::current.popup.colors.overlay );

    window->DrawList->AddRectFilled( bb.Min, bb.Max, ImColor( state.background.get< ImVec4 >( ) ), CThemes::current.popup.rounding );

    if ( icon.has_value( ) )
        window->DrawList->AddText( { bb.Min.x + CThemes::current.popup.padding.x + 2 + GetFontSize( ) / 2 - ( CalcTextSize( icon.value( ).c_str( ) ).x - 1 ) / 2, bb.GetCenter( ).y - CalcTextSize( icon.value( ).c_str( ) ).y / 2 }, Color( text_color ).modulate( ), icon.value( ).c_str( ) );

    window->DrawList->AddText( { bb.Min.x + CThemes::current.popup.padding.x + 2 + ( ( CThemes::current.popup.spaceIcons || icon.has_value( ) ) ? ( CThemes::current.popup.padding.x + 12 ) : 0 ), bb.GetCenter( ).y - label_size.y / 2 }, Color( text_color ).modulate( ), title.c_str( ) );

    auto const key_name = *listening ? "..." : ( *bind == ImGuiKey_None ? "-" : GetKeyName( *bind ) );
    auto const key_size = CalcTextSize( key_name );
    window->DrawList->AddText( { bb.Max.x - key_size.x - 12, bb.GetCenter( ).y - key_size.y / 2 }, Color( text_color ).modulate( ), key_name );

    itemHeightOverride = std::nullopt;
    textColorOverride = std::nullopt;
    popupEnablerCallback = std::nullopt;

    if ( *listening )
    {
        if ( IsKeyPressed( ImGuiKey_Escape ) )
        {
            *listening = false;
            return false;
        }

        for ( int k = ImGuiKey_NamedKey_BEGIN;
              k < ImGuiKey_NamedKey_END; k++ )
        {
            if ( IsKeyPressed( ( ImGuiKey ) k ) )
            {
                *bind = ( ImGuiKey ) k;
                *listening = false;
                break;
            }
        }
    }

    return pressed;
}