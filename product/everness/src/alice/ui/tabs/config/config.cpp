//
// Created by rei on 1/28/2026.
// Rewritten to use local MasonRecode config system.
//

#include "../tabs.hpp"

#include "../../elem/elem.hpp"

#include "animator/animator.hpp"
#include "render/configs.h"

#include <map>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <cstring>

#include "imgui.h"
#include "imgui_internal.h"
#include "fonts/include/font_awesome/font_awesome.hpp"
#include "ui/themes/themes.hpp"

using namespace ImGui;

void alice::tabs::callConfig( int child )
{
    if ( child != 0 )
        return;

    static std::map< int, std::string > cfgList;
    static bool needRefresh = true;
    static int selectedConfig = -1;
    static char newNameBuf[ 64 ] = {};

    PushStyleVar( ImGuiStyleVar_ItemSpacing, { 0, 10 * GetStyle( ).Scale } );

    struct cfg_anim_t
    {
        CAnimator main { float( 0.f ) };
        bool closing = false;
        bool seen = false;
        bool removed_called = false;
    };

    static std::unordered_map< ImGuiID, cfg_anim_t > cfg_anim;

    if ( needRefresh )
    {
        cfgList.clear( );
        cfg_anim.clear( );
        auto list = configs::list_configs( );
        for ( int i = 0; i < ( int ) list.size( ); ++i )
            cfgList[ i ] = list[ i ];
        if ( !cfgList.empty( ) && ( selectedConfig < 0 || selectedConfig >= ( int ) cfgList.size( ) ) )
            selectedConfig = 0;
        else if ( cfgList.empty( ) )
            selectedConfig = -1;
        needRefresh = false;
    }

    // config manager bar
    [ & ]( ) -> void {
        ImGuiWindow* window = GetCurrentWindow( );
        if ( window->SkipItems )
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID label_id = window->GetID( "Config manager" );
        const ImVec2 label_size = CalcTextSize( "Config manager", NULL, true );

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = CalcItemSize( { 0, 0 }, GetContentRegionAvail( ).x, 50 * GetStyle( ).Scale );

        const ImRect bb( pos, pos + size );
        ItemSize( size, style.FramePadding.y );
        if ( !ItemAdd( bb, label_id ) )
            return;

        GetWindowDrawList( )->AddRectFilled( bb.Min, bb.Max, Color( CThemes::current.colors.elem.background ).modulate( ), 12 * GetStyle( ).Scale );

        ImVec2 const center_pos = { bb.Min.x + 10 * GetStyle( ).Scale, bb.GetCenter( ).y };

        auto const btn_size = 30 * GetStyle( ).Scale;
        ImVec2 const start = { bb.Max.x - 10 * GetStyle( ).Scale - ( btn_size * 3 ), bb.GetCenter( ).y - btn_size / 2 };

        struct _state
        {
            CAnimator text { ImVec4( 0, 0, 0, 0 ) };
            CAnimator background { ImVec4( 0, 0, 0, 0 ) };
        };

        static std::unordered_map< ImGuiID, _state > __state;

        GetWindowDrawList( )->AddText( center_pos - ImVec2( 0, label_size.y / 2 ), Color( CThemes::current.colors.text ).modulate( ), "Manager" );

        std::vector< std::pair< std::string, std::function< void( ) > > > inner_buttons = {
            { ICON_FA_ARROWS_ROTATE, [ & ] { needRefresh = true; } },
            { ICON_FA_FOLDER_OPEN, [ & ] { configs::open_config_directory( ); } },
            { ICON_FA_WAND_MAGIC, [ & ] {
                 const static std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
                 std::random_device rd;
                 std::mt19937 generator( rd( ) );
                 std::uniform_int_distribution<> distribution( 0, ( int ) characters.size( ) - 1 );
                 std::string random_string;
                 for ( int i = 0; i < 10; ++i )
                     random_string += characters[ distribution( generator ) ];
                 if ( configs::create_config( random_string.c_str( ) ) )
                     needRefresh = true;
             } } };

        GetWindowDrawList( )->AddRectFilled( start, start + ImVec2( btn_size * inner_buttons.size( ), btn_size ), Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( ), 12 * GetStyle( ).Scale );
        GetWindowDrawList( )->AddRect( start, start + ImVec2( btn_size * inner_buttons.size( ), btn_size ), Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( ), 12 * GetStyle( ).Scale, 0, GetStyle( ).Scale );

        for ( int it { 0 }; it < ( int ) inner_buttons.size( ); it++ )
        {
            auto& btn = inner_buttons[ it ].first;

            ImRect const rect = {
                start + ImVec2( btn_size * it, 0 ), start + ImVec2( btn_size * ( it + 1 ), btn_size ) };

            auto const inner_id = GetID( ( std::to_string( it ) + inner_buttons[ it ].first + "manager" ).c_str( ) );

            ItemAdd( rect, inner_id );

            bool hovered, held;
            bool pressed = ButtonBehavior( rect, inner_id, &hovered, &held, 0 );

            _state& state = __state[ inner_id ];
            state.text.update< ImVec4 >( hovered
                                             ? CThemes::current.colors.text
                                             : CThemes::current.colors.textDim );

            state.background.update< ImVec4 >( hovered ? ImColor( CThemes::current.colors.elem.backgroundOverlay ).Value : Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( 0 ).Value );

            GetWindowDrawList( )->AddText( rect.GetCenter( ) - CalcTextSize( btn.c_str( ) ) / 2,
                                           Color( state.text.get< ImVec4 >( ) ).modulate( ), btn.c_str( ) );

            GetWindowDrawList( )->AddRectFilled( rect.Min, rect.Max, Color( state.background.get< ImVec4 >( ) ).modulate( ), ( it == 0 || it == ( int ) inner_buttons.size( ) - 1 ) ? ( 12 * GetStyle( ).Scale ) : 0, ( it == 0 ) ? ImDrawFlags_RoundCornersLeft : ImDrawFlags_RoundCornersRight );

            if ( it != ( int ) inner_buttons.size( ) - 1 )
                GetWindowDrawList( )->AddLine( { rect.Max.x, rect.Min.y + 5 * GetStyle( ).Scale }, { rect.Max.x, rect.Max.y - 5 * GetStyle( ).Scale }, Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( ), GetStyle( ).Scale );

            if ( pressed )
            {
                state.text.set< ImVec4 >( ImColor( CThemes::current.colors.accent ).Value );
                state.background.set< ImVec4 >( Color( CThemes::current.colors.elem.backgroundOverlay ).modulate( 1.3f * GetStyle( ).Alpha ).Value );
                inner_buttons[ it ].second( );
            }
        }
    }( );

    for ( auto& [ id, st ] : cfg_anim )
        st.seen = false;

    auto const width = GetContentRegionAvail( ).x / 2.f - 5.f * GetStyle( ).Scale;

    std::vector< int > to_remove;
    to_remove.reserve( cfgList.size( ) );

    int it { 0 };
    for ( auto& cfg : cfgList )
    {
        ImGuiID id = GetID( ( cfg.second + std::to_string( cfg.first ) ).c_str( ) );
        auto& st = cfg_anim[ id ];
        st.seen = true;

        float const target = st.closing ? 0.f : 1.f;
        st.main.update< float >( target );

        if ( st.closing && st.main.get< float >( ) <= 0.01f )
        {
            if ( !st.removed_called )
            {
                st.removed_called = true;
                configs::delete_config( cfg.second.c_str( ) );
            }
            to_remove.push_back( cfg.first );
            continue;
        }

        float const a = st.main.get< float >( );
        PushStyleVar( ImGuiStyleVar_Alpha, GetStyle( ).Alpha * a );

        auto const on_delete = [ & ]( ) {
            st.closing = true;
            if ( selectedConfig == cfg.first )
                selectedConfig = -1;
        };

        auto const on_update = [ & ]( ) {
            selectedConfig = cfg.first;
            configs::save_config( cfg.second.c_str( ) );
        };

        auto const on_select = [ & ]( ) {
            selectedConfig = cfg.first;
            configs::load_config( cfg.second.c_str( ) );
        };

        CUIElements::config(
            cfg.second.c_str( ),
            cfg.first,
            ImVec2 { width, 44 * GetStyle( ).Scale },
            ( selectedConfig == cfg.first ),
            on_delete,
            on_select,
            on_update );

        PopStyleVar( );

        if ( ( it % 2 ) == 0 && it != ( int ) cfgList.size( ) - 1 )
            SameLine( 0, 10 * GetStyle( ).Scale );

        it++;
    }

    if ( !to_remove.empty( ) )
    {
        std::sort( to_remove.begin( ), to_remove.end( ) );
        to_remove.erase( std::unique( to_remove.begin( ), to_remove.end( ) ), to_remove.end( ) );
        for ( int i = ( int ) to_remove.size( ) - 1; i >= 0; --i )
        {
            int idx = to_remove[ i ];
            ImGuiID id = GetID( cfgList[ idx ].c_str( ) );
            cfg_anim.erase( id );
            cfgList.erase( idx );
        }
        selectedConfig = -1;
        needRefresh = true;
    }

    {
        std::vector< ImGuiID > garbage;
        garbage.reserve( cfg_anim.size( ) );
        for ( auto& [ id, st ] : cfg_anim )
            if ( !st.seen && st.closing && st.main.get< float >( ) <= 0.01f )
                garbage.push_back( id );
        for ( auto id : garbage )
            cfg_anim.erase( id );
    }

    PopStyleVar( );
}