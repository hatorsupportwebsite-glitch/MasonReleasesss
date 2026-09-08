//
// Created by Kai Tears on 23/01/2026.
//

#include "ui.hpp"

// #include "../blur/blur.hpp"
#include "core.hpp"
#include "../fonts/fonts.hpp"
#include "../fonts/include/font_awesome/font_awesome.hpp"
#include "alerts/alerts.hpp"
#include "assets/assets.hpp"
#include "popups/popups.hpp"
#include "binder/binder.hpp"
#include "tabs/tabs.hpp"
#include "themes/themes.hpp"
#include "widgets/hit_logs/hit_logs.hpp"
#include "widgets/keybinds/keybinds.hpp"
#include "widgets/watermark/watermark.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <ostream>
#include <random>
#include <thread>

using alice::CUIElements;
using alice::CUserInterface;

using namespace ImGui;

// prefs should load here
CUserInterface::CUserInterface( ) = default;
CUserInterface::~CUserInterface( ) = default;

ImTextureID CUserInterface::logo { };
ImTextureID CUserInterface::profile_picture { };

void CUserInterface::renderCategory( Tab* tab ) const
{
    ItemSize( { 0, 0 } );
    SetCursorPosX( 20 );
    TextColored( CThemes::current.colors.tabs.textCategory, "%s", tab->name.c_str( ) );
}

bool CUserInterface::renderTab( Tab* tab, bool active, int width )
{
    auto const cur = GetCursorScreenPos( );
    ImVec2 const btn_sz = { static_cast< float >( width ), 32 };

    bool const ret = InvisibleButton( tab->name.c_str( ), btn_sz );

    auto const hovered = IsItemHovered( );

    ImRect const rect = {
        cur,
        cur + btn_sz };

    tab->background.update( active ? CThemes::current.colors.tabs.backgroundActive : ( hovered ? CThemes::current.colors.tabs.backgroundHovered : CThemes::current.colors.tabs.background ) );

    tab->_icon.update( active ? ImColor( CThemes::current.colors.accent ).Value : ( hovered ? ImColor( CThemes::current.colors.text ).Value : ImColor( CThemes::current.colors.textDim ).Value ) );
    tab->_name.update( active ? ImColor( CThemes::current.colors.text ).Value : ( hovered ? ImColor( CThemes::current.colors.textHover ).Value : ImColor( CThemes::current.colors.textDim ).Value ) );

    auto* draw = GetWindowDrawList( );
    draw->AddRectFilled( rect.Min + ImVec2( 10, 0 ), rect.Max - ImVec2( 10, 0 ), Color( tab->background.get< ImVec4 >( ) ).modulate( ), 6 );
    draw->AddRect( rect.Min + ImVec2( 10, 0 ), rect.Max - ImVec2( 10, 0 ), Color( tab->background.get< ImVec4 >( ) ).modulate( ), 6 );

    auto const icon_size = CalcTextSize( tab->icon.value_or( "0" ).c_str( ) );
    auto const text_size = CalcTextSize( tab->name.c_str( ) );

    if ( tab->icon.has_value( ) )
        draw->AddText( { rect.Min.x + 10 + 10 + GetFontSize( ) / 2 - ( CalcTextSize( tab->icon.value( ).c_str( ) ).x - 1 ) / 2, rect.GetCenter( ).y - icon_size.y / 2 }, Color( tab->_icon.get< ImVec4 >( ) ).modulate( ), tab->icon->c_str( ) );

    draw->AddText( { rect.Min.x + 10 + 10 + 10 + GetFontSize( ), rect.GetCenter( ).y - text_size.y / 2 }, Color( tab->_name.get< ImVec4 >( ) ).modulate( ), tab->name.c_str( ) );

    return ret;
}

bool CUserInterface::renderSubTab( SubTab* subtab, bool active, int idx )
{
    auto const cur = GetCursorScreenPos( );
    auto const text_size = CalcTextSize( subtab->name.c_str( ) );
    auto const width = text_size.x + 14 * 2;
    ImVec2 const btn_sz = { static_cast< float >( width ), GetContentRegionAvail( ).y };

    auto const ret = InvisibleButton( subtab->name.c_str( ), btn_sz );

    auto const hovered = IsItemHovered( );

    ImRect const rect = {
        cur,
        cur + btn_sz };

    subtab->background.update( active ? ImColor( CThemes::current.colors.tabs.backgroundActive ).Value : ( hovered ? ImColor( CThemes::current.colors.tabs.backgroundHovered ).Value : ImColor( CThemes::current.colors.tabs.background ).Value ) );
    subtab->_name.update( active ? ImColor( CThemes::current.colors.text ).Value : ( hovered ? ImColor( CThemes::current.colors.textHover ).Value : ImColor( CThemes::current.colors.textDim ).Value ) );

    auto* draw = GetWindowDrawList( );

    draw->AddRectFilled( rect.Min, rect.Max, Color( CThemes::current.colors.tabs.backgroundHovered ).modulate( ), 6, ( idx == 0 ) ? ImDrawFlags_RoundCornersLeft : ImDrawFlags_RoundCornersRight );
    draw->AddRectFilled( rect.Min, rect.Max, Color( subtab->background.get< ImVec4 >( ) ).modulate( ), 6, ( idx == 0 ) ? ImDrawFlags_RoundCornersLeft : ImDrawFlags_RoundCornersRight );

    draw->AddText( rect.GetCenter( ) - text_size / 2, Color( subtab->_name.get< ImVec4 >( ) ).modulate( ), subtab->name.c_str( ) );

    return ret;
}

void CUserInterface::saveButton( const ImVec2& size, const std::function< void( ) >& save_fn )
{
    auto const cur = GetCursorScreenPos( );

    static CAnimator background { ImVec4( 0, 0, 0, 0 ) };
    static CAnimator text_color { ImVec4( 0, 0, 0, 0 ) };

    if ( InvisibleButton( "Save", size ) )
    {
        save_fn( );
        background.set< ImVec4 >( ImColor( CThemes::current.colors.tabs.backgroundClicked ).Value );
        text_color.set< ImVec4 >( ImColor( CThemes::current.colors.text ).Value );
    }

    ImRect const rect = {
        cur,
        cur + size };

    auto const hovered = IsItemHovered( );

    background.update< ImVec4 >( hovered ? ImColor( CThemes::current.colors.tabs.backgroundHovered ).Value : ImColor( CThemes::current.colors.tabs.background ).Value );
    text_color.update< ImVec4 >( ImColor( CThemes::current.colors.textDim ).Value );

    auto* draw = GetWindowDrawList( );
    draw->AddRectFilled( rect.Min, rect.Max, Color( background.get< ImVec4 >( ) ).modulate( ), 6 );
    draw->AddRect( rect.Min, rect.Max, Color( CThemes::current.colors.border ).modulate( ), 6 );

    const char* text = ICON_FA_FLOPPY_DISK "  Save";
    auto const text_size = CalcTextSize( text );
    draw->AddText( rect.GetCenter( ) - text_size / 2, Color( text_color.get< ImVec4 >( ) ).modulate( ), text );
}

void CUserInterface::applyBlurOverlay( ImGuiWindow* window ) const
{
    if ( !window )
        return;

    if ( m_overlayBlurIntensity.get< float >( ) < 0.01f || m_menuOpacity.get< float >( ) < 0.01f )
        return;

    SetCursorPos( { 0, 0 } );
    BeginChild( "##blur", GetWindowSize( ), 0, ImGuiWindowFlags_NoInputs );
    // g_blur.applyBlur( window->Rect( ), GetWindowDrawList( ), window->WindowRounding, m_overlayBlurIntensity.get< float >( ) * 4, m_menuOpacity.get< float >( ), false );
    EndChild( );
}

void CUserInterface::MenuSettings::render( const std::function< void( ) >& back_task )
{
    auto* window = GetCurrentWindow( );
    auto const rect = window->Rect( );

    main.update< float >( presented ? 1.f : 0.f );

    if ( main.get< float >( ) < 0.001f )
        return;

    auto const alpha = main.get< float >( ) * GetStyle( ).Alpha;

    {
        SetCursorPos( { 180, 0 } );
        BeginChild( "##settings_blur", GetWindowSize( ), 0 );

        // g_blur.applyBlur( { rect.Min + ImVec2( 180, 0 ), rect.Max }, GetWindowDrawList( ), window->WindowRounding, 3.f, alpha, false );

        GetWindowDrawList( )->AddRectFilled( rect.Min + ImVec2( 180, 0 ), rect.Max, Color( CThemes::current.colors.window.settingsOverlay ).modulate( alpha ), window->WindowRounding );
        EndChild( );
    }

    PushStyleVar( ImGuiStyleVar_Alpha, alpha );

    SetCursorPos( { 360, 0 } );

    BeginChild( "##settings", { GetContentRegionAvail( ).x, window->Size.y }, 0 );
    {
        SetCursorPos( { GetWindowSize( ).x - 28 - 10, 10 } );

        {
            static CAnimator gear { ImVec4( 0, 0, 0, 0 ) };

            auto const cur = GetCursorScreenPos( );
            auto const size = ImVec2( 28, 28 );
            if ( InvisibleButton( "##BACK", size ) )
                back_task( );

            auto const hovered = IsItemHovered( );

            ImRect const rect = {
                cur, cur + size };

            gear.update< ImVec4 >( hovered ? ImColor( CThemes::current.colors.text ) : ImColor( CThemes::current.colors.textDim ) );

            GetWindowDrawList( )->AddText( rect.GetCenter( ) - CalcTextSize( ICON_FA_ANGLE_LEFT ) / 2, Color( gear.get< ImVec4 >( ) ).modulate( ), ICON_FA_ANGLE_LEFT );
        }

        SetCursorPos( { 0, 48 } );

        PushStyleVar( ImGuiStyleVar_WindowPadding, { 10, 10 } );

        BeginChild( "##settings_area", GetContentRegionAvail( ), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_AlwaysUseWindowPadding );
        {
            CUIElements::text( "UI" );

            {
                auto& themes = everness::ctx->alice->themes->getAllThemes( );

                static std::vector< std::string > themeNames;
                static std::vector< Theme* > themePtrs;

                themeNames.clear( );
                themePtrs.clear( );

                for ( auto& [ name, theme ] : themes )
                {
                    themeNames.push_back( name );
                    themePtrs.push_back( &theme );
                }

                static int currentThemeIndex = 0;

                for ( int i = 0; i < themePtrs.size( ); ++i )
                {
                    if ( themePtrs[ i ] == &CThemes::current )
                    {
                        currentThemeIndex = i;
                        break;
                    }
                }

                if ( CUIElements::combo( ElemType::Begin, "Theme", &currentThemeIndex, themeNames ) )
                {
                    CThemes::current = *themePtrs[ currentThemeIndex ];
                }
            }

            CUIElements::toggle( ElemType::Middle, "Main glow", &everness::c::get< bool >( everness::ctx->config.uiEnableGlowMain ) );
            CUIElements::toggle( ElemType::Middle, "Logo glow", &everness::c::get< bool >( everness::ctx->config.uiEnableGlowLogo ) );

            CUIElements::color( ElemType::Middle, "Accent color", { &CThemes::current.colors.accent } );

            static CPopup wg( 150, "Widgets", std::nullopt );
            if ( CUIElements::button( ElemType::End, "Widgets" ) && wg.allowOpen( ) )
            {
                wg.setPosition( { GetItemRectMax( ).x - 160, GetItemRectMin( ).y - 40 } );
                wg.opened = true;
            }

            if ( wg.begin( ) )
            {
                PopupEntries::toggle( std::nullopt, "Watermark", &everness::ctx->alice->widgets->watermark->enabled );
                PopupEntries::toggle( std::nullopt, "Keybinds", &everness::ctx->alice->widgets->keybinds->enabled );
                PopupEntries::toggle( std::nullopt, "Hit logs", &everness::ctx->alice->widgets->hitLogs->enabled );

                wg.end( );
            }

            EndChild( );
        }

        PopStyleVar( );

        EndChild( );
    }

    PopStyleVar( );
}

void CUserInterface::init( )
{
    auto& style = GetStyle( );

    style.Scale = 1;
    style.WindowRounding = 12;
    style.WindowBorderSize = 0;
    style.ScrollbarRounding = 12;
    style.ScrollbarSize = 3;

    m_tabs = {
        { .name = "VISUAL",
          .type = TabType::Category },
        {
            .name = "Entities",
            .icon = ICON_FA_USER,
            .type = TabType::Tab,
            .callback = tabs::callVisual,
        },
        { .name = "World", .icon = ICON_FA_GLOBE, .type = TabType::Tab, .callback = tabs::callWorld,
          .subTabs = std::vector< SubTab >{ { "Visual", tabs::callWorld }, { "Misc", tabs::callWorldMisc } } },

        { .name = "COMBAT", .type = TabType::Category },
        { .name = "Legitbot", .icon = ICON_FA_DROPLET, .type = TabType::Tab, .callback = tabs::callLegitbot,
          .subTabs = std::vector< SubTab >{ { "Aimbot", tabs::callLegitbot }, { "Triggerbot", tabs::callLegitbotTrigger } } },

        { .name = "CLOUD", .type = TabType::Category },
        { .name = "Skins", .icon = ICON_FA_SHIRT, .type = TabType::Tab, .callback = tabs::callSkins },
        { .name = "Configs", .icon = ICON_FA_CLOUD, .type = TabType::Tab, .callback = tabs::callConfig } };

    m_selectedTab = 1;
    m_pendingTab = 1;
    m_callback = m_tabs[ m_selectedTab ].callback;

    everness::ctx->alice->themes->addDefaults( );
}

void CUserInterface::render( )
{
    PushStyle padding( ImGuiStyleVar_WindowPadding, ImVec2 { 0, 0 } );

    auto& io = GetIO( );
    auto& style = GetStyle( );

    style.Colors[ ImGuiCol_WindowBg ] = CThemes::current.colors.window.background;
    style.Colors[ ImGuiCol_ScrollbarGrab ] = CThemes::current.colors.scrollbar.grab;
    style.Colors[ ImGuiCol_ScrollbarGrabHovered ] = CThemes::current.colors.scrollbar.grab;
    style.Colors[ ImGuiCol_ScrollbarGrabActive ] = CThemes::current.colors.scrollbar.grab;
    style.Colors[ ImGuiCol_ScrollbarBg ] = CThemes::current.colors.scrollbar.background;
    style.Colors[ ImGuiCol_Text ] = CThemes::current.colors.text;

    m_overlayBlurIntensity.update< float >( m_presented ? 0.f : 1.f, 0.25f );

    if ( m_transition.active )
    {
        if ( m_transition.target == TransitionTarget::Tab )
        {
            m_subTabsAreaOpacity.set< float >( m_transition.alpha.get< float >( ) );
            m_mainAreaOpacity.set< float >( m_transition.alpha.get< float >( ) );
            m_mainAreaInnerBlur.set< float >( m_transition.alpha.get< float >( ) );
        } else if ( m_transition.target == TransitionTarget::SubTab )
        {
            m_mainAreaOpacity.set< float >( m_transition.alpha.get< float >( ) );
            m_mainAreaInnerBlur.set< float >( m_transition.alpha.get< float >( ) );
        }
    } else
    {
        m_subTabsAreaOpacity.update< float >( 1.f );
        m_mainAreaOpacity.update< float >( 1.f );
        m_mainAreaInnerBlur.update< float >( 1.f, 0.5f );
    }

    m_menuOpacity.update< float >( ( m_presented || m_overlayBlurIntensity.get< float >( ) < 0.4f ) ? 1.f : 0.f );
    auto const menu_opacity = m_menuOpacity.get< float >( );

    if ( menu_opacity < 0.001f )
        return;

    SetNextWindowSize( m_windowSize );
    SetNextWindowPos( io.DisplaySize / 2 - m_windowSize / 2, ImGuiCond_Once );

    PushStyle opacity( ImGuiStyleVar_Alpha, menu_opacity );

    Begin( "@alicegang", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus );
    m_window = GetCurrentWindow( );

    m_allowEdits = !IsWindowHovered( ) && !m_window->Rect( ).Contains( GetMousePos( ) );

    {
        ImVec2 shadow_offset = ImVec2( ImCos( IM_PI * 0.25f ), ImSin( IM_PI * 0.25f ) ) * 0;
        GetBackgroundDrawList( )->AddShadowRect( m_window->Pos, m_window->Pos + m_window->Size, Color( CThemes::current.colors.window.shadow ).modulate( ), 64, shadow_offset, 0, m_window->WindowRounding * 4 );
    }

    // const //

    int const sidebar_width = 180;

    // transitions //

    m_transition.alpha.update( m_transition.active ? 0.f : 1.f );

    if ( m_transition.active && m_transition.alpha.get< float >( ) < 0.01f )
    {
        if ( m_transition.apply )
            m_transition.apply( );

        m_transition.active = false;
        m_transition.target = TransitionTarget::None;
    }

    // start //
    {
        m_window->DrawList->PushClipRectFullScreen( );
        ImVec2 shadow_offset = ImVec2( ImCos( IM_PI * 0.25f ), ImSin( IM_PI * 0.25f ) ) * 0;
        m_window->DrawList->AddShadowRect( m_window->Pos + ImVec2( sidebar_width, 0 ), m_window->Pos + m_windowSize, Color( CThemes::current.colors.window.overlayShadow ).modulate( ), 48, shadow_offset, 0, 64 );
        m_window->DrawList->PopClipRect( );
    }

    // m_window->DrawList->AddRectFilled( m_window->Pos + ImVec2( sidebar_width, 0 ), m_window->Pos + m_windowSize, Color( 20, 20, 20, 100 ).modulate( ), style.WindowRounding );
    m_window->DrawList->AddRect( m_window->Pos + ImVec2( sidebar_width, 0 ), m_window->Pos + m_windowSize, Color( CThemes::current.colors.window.overlayOutline ).modulate( ), style.WindowRounding );

    // m_window->DrawList->AddLine( m_window->Pos + ImVec2( sidebar_width, 0 ), m_window->Pos + ImVec2( sidebar_width, m_windowSize.y ), Color( 30, 30, 30 ).modulate( ), 1 );

    // CUIElements::applyLiquidGlass( [ & ]( ImDrawList* draw ) -> void {
    //     draw->AddRect( m_window->Rect( ).Min, m_window->Rect( ).Max, Color( 30, 30, 30 ).modulate( ), style.WindowRounding );
    // },
    //                   m_window->DrawList, m_window->Rect( ), ImColor( 30, 30, 30 ), ImColor( 64, 64, 64 ) );

    // sidebar //

    BeginChild( "##title", { sidebar_width, 80 }, 0 );
    {
        ImRect const area = GetCurrentWindow( )->Rect( );
        ImVec2 const image_size = { 80, 80 };

        if ( everness::c::get< bool >( everness::ctx->config.uiEnableGlowLogo ) )
        {
            GetWindowDrawList( )->PushClipRect( m_window->Rect( ).Min, m_window->Rect( ).Max, false );
            GetWindowDrawList( )->AddShadowCircle( area.GetCenter( ), 4, Color( CThemes::current.colors.accent ).modulate( 200.f / 255.f * GetStyle( ).Alpha ), 240, { 0, 0 }, 0, 100 );
            GetWindowDrawList( )->PopClipRect( );
        }

        int vert_start_idx = GetWindowDrawList( )->VtxBuffer.Size;
        GetWindowDrawList( )->AddImage( logo, area.GetCenter( ) - image_size / 2 + ImVec2( 0, 10 ), area.GetCenter( ) + image_size / 2 + ImVec2( 0, 10 ), { 0, 0 }, { 1, 1 }, Color( 255, 255, 255 ).modulate( ) );
        int vert_end_idx = GetWindowDrawList( )->VtxBuffer.Size;

        ShadeVertsLinearColorGradientKeepAlpha(
            GetWindowDrawList( ),
            vert_start_idx,
            vert_end_idx,
            area.GetCenter( ) - ImVec2( image_size.x / 2, 0 ),
            area.GetCenter( ) + ImVec2( image_size.x / 2, 0 ),
            ImColor( 255, 255, 255 ),
            Color( CThemes::current.colors.accent ).u32( ) );

        EndChild( );
    }

    // tabs //

    SetCursorPos( { 0, 80 } );
    BeginChild( "##tabs", { sidebar_width, GetContentRegionAvail( ).y }, 0 );
    {
        PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2 { 0, 5 } );

        for ( int it { 0 }; it < m_tabs.size( ); ++it )
        {
            auto& tab = m_tabs[ it ];

            switch ( tab.type )
            {
            case TabType::Category:
                renderCategory( &tab );
                break;
            case TabType::Tab:
                if ( renderTab( &tab, it == m_pendingTab, sidebar_width ) )
                {
                    m_transition = {
                        .alpha = 1.f,
                        .active = true,
                        .target = TransitionTarget::Tab,
                        .apply = [ this, it, tab ]( ) {
                            m_selectedTab = it;
                            m_callback = tab.callback;
                        } };

                    m_settings.presented = false;
                    m_pendingTab = it;
                    m_selectedSubTab = 0;
                }
                break;
            default:
                break;
            }
        }

        PopStyleVar( );

        EndChild( );
    }

    // top area //

    PushStyleVar( ImGuiStyleVar_WindowPadding, { 10, 10 } );

    SetCursorPos( { sidebar_width + 1, 1 } );
    BeginChild( "##top_area", { GetContentRegionAvail( ).x, 48 }, 0, ImGuiWindowFlags_AlwaysUseWindowPadding );
    {
        if ( m_selectedTab == 1 )
        {
            saveButton( { 70, GetContentRegionAvail( ).y }, [] {
                everness::ctx->alice->alerts->create( "Error", { 260 * GetStyle( ).Scale, 200 * GetStyle( ).Scale }, []( Alert& alert ) {
                    using namespace ImGui;

                    PushStyleColor( ImGuiCol_Text, ImColor( CThemes::current.colors.textDim ).Value );
                    CAlerts::renderContents( GetCursorScreenPos( ) + ImVec2( 5 * GetStyle( ).Scale, 0 ), "Something went wrong...", nullptr, GetWindowSize( ).x - 10 * GetStyle( ).Scale );
                    PopStyleColor( );

                    SetCursorPosY( GetWindowSize( ).y - 44 * GetStyle( ).Scale );

                    if ( CAlerts::button( "OK", { 240 * GetStyle( ).Scale, 44 * GetStyle( ).Scale }, true ) )
                        alert.close( );
                } );
            } );

            SameLine( 0, 10 );
        }

        PushStyleVar( ImGuiStyleVar_Alpha, m_subTabsAreaOpacity.get< float >( ) * menu_opacity );

        for ( int it { 0 }; it < m_tabs[ m_selectedTab ].subTabs.value_or( std::vector< SubTab > { } ).size( ); it++ )
        {
            auto& subtab = m_tabs[ m_selectedTab ].subTabs.value( )[ it ];

            if ( renderSubTab( &subtab, it == m_pendingSubTab, it ) )
            {
                m_transition = {
                    .alpha = 1.f,
                    .active = true,
                    .target = TransitionTarget::SubTab,
                    .apply = [ this, it, subtab ]( ) {
                        m_selectedSubTab = it;
                        m_callback = subtab.callback;
                    } };

                m_pendingSubTab = it;
            }

            SameLine( 0, 0 );
        }

        PopStyleVar( );

        SetCursorPos( { GetWindowSize( ).x - 28 - 10, 10 } );

        {
            static CAnimator gear { ImVec4( 0, 0, 0, 0 ) };

            auto const cur = GetCursorScreenPos( );
            auto const size = GetContentRegionAvail( );
            if ( InvisibleButton( "##SETTINGS", size ) )
                m_settings.presented = true;

            auto const hovered = IsItemHovered( );

            ImRect const rect = {
                cur, cur + size };

            gear.update< ImVec4 >( hovered ? ImColor( CThemes::current.colors.text ) : ImColor( CThemes::current.colors.textDim ) );

            GetWindowDrawList( )->AddText( rect.GetCenter( ) - CalcTextSize( ICON_FA_GEAR ) / 2, Color( gear.get< ImVec4 >( ) ).modulate( ), ICON_FA_GEAR );
        }

        EndChild( );
    }

    m_window->DrawList->AddLine( { m_window->Pos.x + sidebar_width, m_window->Pos.y + 50 }, { m_window->Pos.x + m_window->Size.x, m_window->Pos.y + 50 }, Color( CThemes::current.colors.border ).modulate( ) );

    PopStyleVar( );

    // main area //

    SetCursorPos( { sidebar_width + 1, 51 + 30 * ( 1.0f - m_mainAreaOpacity.get< float >( ) ) } );

    PushStyleVar( ImGuiStyleVar_Alpha, m_mainAreaOpacity.get< float >( ) * menu_opacity );

    if ( everness::c::get< bool >( everness::ctx->config.uiEnableGlowMain ) )
    {
        GetWindowDrawList( )->PushClipRect( m_window->Rect( ).Min, m_window->Rect( ).Max, false );
        GetWindowDrawList( )->AddShadowCircle( { m_window->Pos.x + m_windowSize.x / 2 + 80, m_window->Pos.y + 10 }, 4, Color( CThemes::current.colors.accent ).modulate( 80.f / 255.f * GetStyle( ).Alpha ), 666, { 0, 0 }, 0, 100 );
        GetWindowDrawList( )->PopClipRect( );
    }

    BeginChild( "##main_area", GetContentRegionAvail( ), 0 );
    {
        PushStyleVar( ImGuiStyleVar_WindowPadding, { 0, 0 } );

        if ( m_selectedTab == kConfigsTab )
        {
            SetCursorPos( { 10, 10 } );
            BeginChild( "##center", { GetContentRegionAvail( ).x - 10, 0 }, 0 | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding );
            {
                if ( m_callback )
                    m_callback( 0 );

                EndChild( );
            }
        } else
        {
            SetCursorPos( { 10, 10 } );
            BeginChild( "##left", { GetContentRegionAvail( ).x / 2 - 10, 0 }, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding );
            {
                if ( m_callback )
                    m_callback( 1 );
                EndChild( );
            }

            SameLine( 0, 10 );

            BeginChild( "##right", { GetContentRegionAvail( ).x - 10, 0 }, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding );
            {
                if ( m_callback )
                    m_callback( 2 );
                EndChild( );
            }
        }

        PopStyleVar( );

        ItemSize( { 0, 10 } );

        if ( auto const blur_value = 1.f - m_mainAreaInnerBlur.get< float >( ); blur_value > 0.01f )
        {
            SetCursorPos( { 0, 0 } );
            BeginChild( "##inner_blur_overlay", GetWindowSize( ), 0, ImGuiWindowFlags_NoInputs );
            {
                PushStyleVar( ImGuiStyleVar_Alpha, 1 );

                // g_blur.applyBlur( GetCurrentWindow( )->Rect( ), GetWindowDrawList( ), style.WindowRounding, 1.f, blur_value, false );

                PopStyleVar( );

                EndChild( );
            }
        }

        EndChild( );
    }

    PopStyleVar( );

    // end //

    m_settings.render( [ this ] {
        m_settings.presented = false;
    } );

    applyBlurOverlay( m_window );

    End( );
}

bool& CUserInterface::isPresented( )
{
    return m_presented;
}
