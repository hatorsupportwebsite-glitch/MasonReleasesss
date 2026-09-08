#include "core.hpp"
#include "../tabs.hpp"

#include "../../elem/elem.hpp"
#include "../../popups/popups.hpp"
#include "../../themes/themes.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "ui/binder/binder.hpp"
#include "fonts/include/font_awesome/font_awesome.hpp"

#include "ui_state.hpp"
#include "render/kill_sound.h"

using namespace ImGui;
using namespace alice;
using namespace everness;

static void esp_popup_gradient_anim_lr( bool& anim_left, bool& anim_right )
{
    PopupEntries::toggle( std::nullopt, "Left Animated", &anim_left );
    if ( anim_left )
        anim_right = false;
    PopupEntries::toggle( std::nullopt, "Right Animated", &anim_right );
    if ( anim_right )
        anim_left = false;
}

// ─── Entities ────────────────────────────────────────────────

void alice::tabs::callVisual( int child )
{
    if ( child == 1 )
    {
        static CPopup pop_hp( 170, "Health", std::nullopt );
        static CPopup pop_arm( 170, "Armor", std::nullopt );
        static CPopup pop_af( 170, "Armor Flag", std::nullopt );
        static CPopup pop_nick( 170, "Nicknames", std::nullopt );
        static CPopup pop_money( 170, "Money", std::nullopt );
        static CPopup pop_ping( 170, "Ping", std::nullopt );
        static CPopup pop_wpn( 170, "Weapon", std::nullopt );
        static CPopup pop_chams( 180, "Chams", std::nullopt );
        static CPopup pop_glow( 170, "Glow", std::nullopt );

        CUIElements::toggle( ElemType::Begin, "Health##ent_hp", &ui_state::entities::g_esp_hp,
            { (ImVec4*)ui_state::entities::g_esp_color_hp_full, (ImVec4*)ui_state::entities::g_esp_color_hp_low },
            std::make_pair( &pop_hp, [&]( ) {
                if ( pop_hp.begin( ) )
                {
                    PopupEntries::toggle( std::nullopt, "Glow Gradient", &ui_state::entities::g_esp_hp_glow_gradient );
                    PopupEntries::toggle( std::nullopt, "Text", &ui_state::entities::g_esp_hp_show_text );
                    PopupEntries::toggle( std::nullopt, "Glow", &ui_state::entities::g_esp_hp_glow );
                    pop_hp.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Armor##ent_arm", &ui_state::entities::g_esp_armor,
            { (ImVec4*)ui_state::entities::g_esp_color_armor, (ImVec4*)ui_state::entities::g_esp_color_armor_low },
            std::make_pair( &pop_arm, [&]( ) {
                if ( pop_arm.begin( ) )
                {
                    PopupEntries::toggle( std::nullopt, "Glow Gradient", &ui_state::entities::g_esp_armor_glow_gradient );
                    PopupEntries::toggle( std::nullopt, "Text", &ui_state::entities::g_esp_armor_show_text );
                    PopupEntries::toggle( std::nullopt, "Glow", &ui_state::entities::g_esp_armor_glow );
                    pop_arm.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Armor Flag##ent_af", &ui_state::entities::g_esp_armor_flag_force,
            { (ImVec4*)ui_state::entities::g_esp_color_armor_flag, (ImVec4*)ui_state::entities::g_esp_color_armor_flag_grad },
            std::make_pair( &pop_af, [&]( ) {
                if ( pop_af.begin( ) )
                {
                    esp_popup_gradient_anim_lr( ui_state::entities::g_esp_armor_flag_gradient_anim_left, ui_state::entities::g_esp_armor_flag_gradient_anim_right );
                    pop_af.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Nicknames##ent_nick", &ui_state::entities::g_esp_name,
            { (ImVec4*)ui_state::entities::g_esp_color_name, (ImVec4*)ui_state::entities::g_esp_color_name_grad },
            std::make_pair( &pop_nick, [&]( ) {
                if ( pop_nick.begin( ) )
                {
                    esp_popup_gradient_anim_lr( ui_state::entities::g_esp_name_gradient_anim_left, ui_state::entities::g_esp_name_gradient_anim_right );
                    pop_nick.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Avatars##ent_av", &ui_state::entities::g_esp_avatar );

        CUIElements::toggle( ElemType::Middle, "Money##ent_mon", &ui_state::entities::g_esp_money,
            { (ImVec4*)ui_state::entities::g_esp_color_money, (ImVec4*)ui_state::entities::g_esp_color_money_grad },
            std::make_pair( &pop_money, [&]( ) {
                if ( pop_money.begin( ) )
                {
                    esp_popup_gradient_anim_lr( ui_state::entities::g_esp_money_gradient_anim_left, ui_state::entities::g_esp_money_gradient_anim_right );
                    pop_money.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Ping##ent_png", &ui_state::entities::g_esp_ping,
            { (ImVec4*)ui_state::entities::g_esp_color_ping, (ImVec4*)ui_state::entities::g_esp_color_ping_grad },
            std::make_pair( &pop_ping, [&]( ) {
                if ( pop_ping.begin( ) )
                {
                    esp_popup_gradient_anim_lr( ui_state::entities::g_esp_ping_gradient_anim_left, ui_state::entities::g_esp_ping_gradient_anim_right );
                    pop_ping.end( );
                }
            } ) );

        static CPopup pop_dev( 170, "Device", std::nullopt );
        CUIElements::toggle( ElemType::Middle, "Device##ent_dev", &ui_state::entities::g_esp_device_enabled,
            { (ImVec4*)ui_state::entities::g_esp_color_device, (ImVec4*)ui_state::entities::g_esp_color_device_grad },
            std::make_pair( &pop_dev, [&]( ) {
                if ( pop_dev.begin( ) )
                {
                    esp_popup_gradient_anim_lr( ui_state::entities::g_esp_device_gradient_anim_left, ui_state::entities::g_esp_device_gradient_anim_right );
                    pop_dev.end( );
                }
            } ) );

        static CPopup pop_switching( 170, "Switching", std::nullopt );
        CUIElements::toggle( ElemType::Middle, "Switching##ent_sw", &ui_state::entities::g_esp_switching,
            { (ImVec4*)ui_state::entities::g_esp_color_switching, (ImVec4*)ui_state::entities::g_esp_color_switching_grad },
            std::make_pair( &pop_switching, [&]( ) {
                if ( pop_switching.begin( ) )
                {
                    esp_popup_gradient_anim_lr( ui_state::entities::g_esp_switching_gradient_anim_left, ui_state::entities::g_esp_switching_gradient_anim_right );
                    pop_switching.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Weapon##ent_wpn", &ui_state::entities::g_esp_weapon,
            { (ImVec4*)ui_state::entities::g_esp_color_weapon, (ImVec4*)ui_state::entities::g_esp_color_weapon_grad },
            std::make_pair( &pop_wpn, [&]( ) {
                if ( pop_wpn.begin( ) )
                {
                    PopupEntries::toggle( std::nullopt, "Icon", &ui_state::entities::g_esp_weapon_icon );
                    PopupEntries::toggle( std::nullopt, "Text", &ui_state::entities::g_esp_weapon_text );
                    esp_popup_gradient_anim_lr( ui_state::entities::g_esp_weapon_gradient_anim_left, ui_state::entities::g_esp_weapon_gradient_anim_right );
                    pop_wpn.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Skeleton##ent_skel", &ui_state::entities::g_esp_skeleton,
            { (ImVec4*)ui_state::entities::g_esp_color_skeleton } );

        CUIElements::toggle( ElemType::Middle, "Chams##ent_chams", &ui_state::entities::g_esp_hitbox_outline_fill,
            { (ImVec4*)ui_state::entities::g_esp_color_hitbox_fill, (ImVec4*)ui_state::entities::g_esp_color_hitbox_fill_bottom },
            std::make_pair( &pop_chams, [&]( ) {
                if ( pop_chams.begin( ) )
                {
                    esp_popup_gradient_anim_lr( ui_state::entities::g_esp_chams_gradient_anim_left, ui_state::entities::g_esp_chams_gradient_anim_right );
                    pop_chams.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::End, "Glow##ent_glow", &ui_state::entities::g_esp_hitbox_glow,
            { (ImVec4*)ui_state::entities::g_esp_glow_color, (ImVec4*)ui_state::entities::g_esp_glow_color_bottom },
            std::make_pair( &pop_glow, [&]( ) {
                if ( pop_glow.begin( ) )
                {
                    esp_popup_gradient_anim_lr( ui_state::entities::g_esp_glow_gradient_anim_left, ui_state::entities::g_esp_glow_gradient_anim_right );
                    pop_glow.end( );
                }
            } ) );
    }
    else if ( child == 2 )
    {
        static CPopup pop_box( 170, "Box", std::nullopt );
        static CPopup pop_snap( 170, "Snaplines", std::nullopt );
        static CPopup pop_tracer( 170, "Bullet Tracer", std::nullopt );
        static CPopup pop_hitmarker( 170, "Hit Marker", std::nullopt );
        static CPopup pop_hitsparks( 180, "Hit Sparks", std::nullopt );
        static CPopup pop_killsound( 220, "Kill Sound", std::nullopt );
        static CPopup pop_soul( 190, "Soul Animation", std::nullopt );
        static CPopup pop_death( 170, "Death Animation", std::nullopt );
        auto draw_fade_controls = []( float* duration ) {
            if ( !duration ) return;
            PopupEntries::sliderFloat( std::nullopt, "Fade Speed", duration, 0.1f, 4.0f );
            PopupEntries::separator( );
            if ( PopupEntries::button( std::nullopt, "1.0s" ) ) *duration = 1.0f;
            if ( PopupEntries::button( std::nullopt, "1.5s" ) ) *duration = 1.5f;
            if ( PopupEntries::button( std::nullopt, "2.0s" ) ) *duration = 2.0f;
            if ( PopupEntries::button( std::nullopt, "2.5s" ) ) *duration = 2.5f;
        };
        CUIElements::toggle( ElemType::Begin, "Box##ent2_box", &ui_state::entities::g_esp_box,
            { (ImVec4*)ui_state::entities::g_esp_color_box, (ImVec4*)ui_state::entities::g_esp_color_box_grad },
            std::make_pair( &pop_box, [&]( ) {
                if ( pop_box.begin( ) )
                {
                    esp_popup_gradient_anim_lr( ui_state::entities::g_esp_box_gradient_anim_left, ui_state::entities::g_esp_box_gradient_anim_right );
                    pop_box.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Corner Box##ent2_corner", &ui_state::entities::g_esp_corner_box );

        CUIElements::toggle( ElemType::Middle, "Filled Box##ent2_fill", &ui_state::entities::g_esp_filled,
            { (ImVec4*)ui_state::entities::g_esp_color_hitbox_fill } );

        CUIElements::toggle( ElemType::Middle, "Snaplines##ent2_snap", &ui_state::entities::g_esp_snaplines_top,
            { (ImVec4*)ui_state::entities::g_esp_color_snaplines },
            std::make_pair( &pop_snap, [&]( ) {
                if ( pop_snap.begin( ) )
                {
                    PopupEntries::toggle( std::nullopt, "Top", &ui_state::entities::g_esp_snaplines_top );
                    PopupEntries::toggle( std::nullopt, "Bottom", &ui_state::entities::g_esp_snaplines_bottom );
                    pop_snap.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Step Circle##ent2_circle", &ui_state::entities::g_esp_circle,
            { (ImVec4*)ui_state::entities::g_esp_color_circle } );

        CUIElements::toggle( ElemType::Middle, "Arrows##ent2_arrows", &ui_state::entities::g_esp_oof_arrows,
            { (ImVec4*)ui_state::entities::g_esp_color_arrows } );

        CUIElements::toggle( ElemType::Middle, "Bullet Tracer##ent2_tracer", &ui_state::entities::g_esp_bullet_tracer,
            { (ImVec4*)ui_state::entities::g_esp_color_bullet_tracer },
            std::make_pair( &pop_tracer, [&]( ) {
                if ( pop_tracer.begin( ) )
                {
                    draw_fade_controls( &ui_state::entities::g_esp_bullet_tracer_duration_sec );
                    pop_tracer.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Smooth Elements##ent2_smooth", &ui_state::entities::g_esp_smooth_elements );

        CUIElements::toggle( ElemType::Middle, "Hit Marker##ent2_hitmarker", &ui_state::entities::g_esp_hit_marker,
            { (ImVec4*)ui_state::entities::g_esp_color_hit_marker, (ImVec4*)ui_state::entities::g_esp_color_hit_marker_grad },
            std::make_pair( &pop_hitmarker, [&]( ) {
                if ( pop_hitmarker.begin( ) )
                {
                    draw_fade_controls( &ui_state::entities::g_esp_hit_marker_duration_sec );
                    pop_hitmarker.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Hit Sparks##ent2_hitsparks", &ui_state::entities::g_esp_hit_sparks,
            { (ImVec4*)ui_state::entities::g_esp_color_hit_sparks, (ImVec4*)ui_state::entities::g_esp_color_hit_sparks_grad },
            std::make_pair( &pop_hitsparks, [&]( ) {
                if ( pop_hitsparks.begin( ) )
                {
                    PopupEntries::sliderFloat( std::nullopt, "Speed", &ui_state::entities::g_esp_hit_sparks_duration_sec, 0.15f, 2.00f );
                    PopupEntries::sliderInt( std::nullopt, "Count", &ui_state::entities::g_esp_hit_sparks_count, 4, 80 );
                    PopupEntries::sliderFloat( std::nullopt, "Spread", &ui_state::entities::g_esp_hit_sparks_spread, 0.10f, 2.00f );
                    pop_hitsparks.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Kill Sound##ent2_killsnd", &ui_state::entities::g_esp_kill_sound, { },
            std::make_pair( &pop_killsound, [&]( ) {
                if ( pop_killsound.begin( ) )
                {
                    static std::vector< KillSoundItem > kill_items;
                    kill_sound::scan_folder( kill_items );
                    if ( !kill_items.empty( ) )
                    {
                        if ( ui_state::entities::g_esp_kill_sound_index < 0 || ui_state::entities::g_esp_kill_sound_index >= (int)kill_items.size( ) )
                            ui_state::entities::g_esp_kill_sound_index = 0;
                    }
                    float vol_pct = ui_state::entities::g_esp_kill_sound_volume * 100.f;
                    PopupEntries::sliderFloat( std::nullopt, "Volume %", &vol_pct, 0.f, 100.f );
                    ui_state::entities::g_esp_kill_sound_volume = vol_pct / 100.f;
                    if ( ui_state::entities::g_esp_kill_sound_volume < 0.f ) ui_state::entities::g_esp_kill_sound_volume = 0.f;
                    if ( ui_state::entities::g_esp_kill_sound_volume > 1.f ) ui_state::entities::g_esp_kill_sound_volume = 1.f;
                    PopupEntries::separator( );
                    if ( !kill_items.empty( ) )
                    {
                        PushStyleColor( ImGuiCol_ChildBg, Color( CThemes::current.popup.colors.background ).modulate( ).Value );
                        BeginChild( "##killsnd_list", ImVec2( 0, 140 ), ImGuiChildFlags_None, ImGuiWindowFlags_AlwaysVerticalScrollbar );
                        ImGuiListClipper clipper;
                        clipper.Begin( (int)kill_items.size( ) );
                        clipper.IncludeItemByIndex( ui_state::entities::g_esp_kill_sound_index );
                        while ( clipper.Step( ) )
                        {
                            for ( int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i )
                            {
                                PushID( i );
                                const bool item_selected = ( ui_state::entities::g_esp_kill_sound_index == i );
                                bool v = item_selected;
                                if ( PopupEntries::toggle( std::nullopt, kill_items[ (size_t)i ].label_utf8, &v ) && ui_state::entities::g_esp_kill_sound_index != i )
                                    ui_state::entities::g_esp_kill_sound_index = i;
                                if ( item_selected )
                                    SetItemDefaultFocus( );
                                PopID( );
                            }
                        }
                        EndChild( );
                        PopStyleColor( );
                    }
                    else
                    {
                        PopupEntries::textColorOverride = ImColor( CThemes::current.colors.textDim );
                        PopupEntries::button( std::nullopt, "No .wav in Documents\\MasonRecode\\killsound" );
                        PopupEntries::textColorOverride = std::nullopt;
                    }
                    pop_killsound.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Soul Animation##ent2_soul", &ui_state::entities::g_soul_animation,
            { (ImVec4*)ui_state::entities::g_soul_color_top, (ImVec4*)ui_state::entities::g_soul_color_bottom },
            std::make_pair( &pop_soul, [&]( ) {
                if ( pop_soul.begin( ) )
                {
                    PopupEntries::toggle( std::nullopt, "Gradient", &ui_state::entities::g_soul_gradient );
                    esp_popup_gradient_anim_lr( ui_state::entities::g_soul_gradient_anim_left, ui_state::entities::g_soul_gradient_anim_right );
                    PopupEntries::separator( );
                    PopupEntries::sliderFloat( std::nullopt, "Duration", &ui_state::entities::g_soul_lifetime_sec, 0.5f, 8.0f );
                    pop_soul.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::End, "Death Animation##ent2_death", &ui_state::entities::g_kill_animated, { },
            std::make_pair( &pop_death, [&]( ) {
                if ( pop_death.begin( ) )
                {
                    draw_fade_controls( &ui_state::entities::g_kill_fade_duration_sec );
                    pop_death.end( );
                }
            } ) );
    }
}

// ─── World > Visual ──────────────────────────────────────────

void alice::tabs::callWorld( int child )
{
    if ( child == 1 )
    {
        static CPopup pop_gren_trails( 180, "Grenade Trails", std::nullopt );
        CUIElements::toggle( ElemType::Begin, "Grenade##w_gren", &ui_state::entities::g_esp_grenades,
            { (ImVec4*)ui_state::entities::g_esp_grenade_icon_color, (ImVec4*)ui_state::entities::g_esp_grenade_icon_color_grad } );
        CUIElements::toggle( ElemType::Middle, "Show Timer##w_gren_timer", &ui_state::entities::g_esp_grenade_show_timer );
        CUIElements::toggle( ElemType::Middle, "Show Radius##w_gren_radius", &ui_state::entities::g_esp_grenade_show_radius );
        CUIElements::toggle( ElemType::Middle, "Grenade Particles##w_gren_particles", &ui_state::entities::g_esp_grenade_particles );
        CUIElements::toggle( ElemType::Middle, "Grenade Trails##w_gren_trails", &ui_state::entities::g_esp_grenade_trails,
            { (ImVec4*)ui_state::entities::g_esp_grenade_trail_color, (ImVec4*)ui_state::entities::g_esp_grenade_trail_color_grad },
            std::make_pair( &pop_gren_trails, [&]( ) {
                if ( pop_gren_trails.begin( ) )
                {
                    PopupEntries::sliderFloat( std::nullopt, "Trail", &ui_state::entities::g_esp_grenade_trail_length_sec, 0.08f, 2.00f );
                    PopupEntries::toggle( std::nullopt, "Trail Particles", &ui_state::entities::g_esp_grenade_trail_particles );
                    pop_gren_trails.end( );
                }
            } ) );

        CUIElements::color( ElemType::Middle, "Smoke Color##w_smoke",
            { (ImVec4*)ui_state::entities::g_esp_smoke_circle_outline_color } );

        CUIElements::color( ElemType::End, "Molotov Color##w_molo",
            { (ImVec4*)ui_state::entities::g_esp_molotov_circle_outline_color } );
    }
    else if ( child == 2 )
    {
        static CPopup pop_fog( 190, "Custom Fog", std::nullopt );
        static CPopup pop_sky( 180, "Sky", std::nullopt );
        static CPopup pop_tint( 190, "World Tint", std::nullopt );

        CUIElements::toggle( ElemType::Begin, "Night Mode##w_night", &ui_state::world::night_mode );

        CUIElements::toggle( ElemType::Middle, "Custom Fog##w_fog", &ui_state::world::world_fog_enable,
            { (ImVec4*)ui_state::world::world_fog_color },
            std::make_pair( &pop_fog, [&]( ) {
                if ( pop_fog.begin( ) )
                {
                    PopupEntries::sliderFloat( std::nullopt, "Start", &ui_state::world::world_fog_start, 0.f, 200.f );
                    PopupEntries::sliderFloat( std::nullopt, "End", &ui_state::world::world_fog_end, 1.f, 300.f );
                    PopupEntries::sliderFloat( std::nullopt, "Density", &ui_state::world::world_fog_density, 0.001f, 0.25f );
                    PopupEntries::toggle( std::nullopt, "Debug Logs", &ui_state::world::world_fog_debug );
                    pop_fog.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Sky Color##w_sky", &ui_state::world::world_sky_enable,
            { (ImVec4*)ui_state::world::world_sky_color },
            std::make_pair( &pop_sky, [&]( ) {
                if ( pop_sky.begin( ) )
                {
                    PopupEntries::sliderFloat( std::nullopt, "Intensity", &ui_state::world::world_sky_intensity, 0.1f, 4.0f );
                    pop_sky.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "World Tint##w_tint", &ui_state::world::world_tint_enable,
            { (ImVec4*)ui_state::world::world_tint_color },
            std::make_pair( &pop_tint, [&]( ) {
                if ( pop_tint.begin( ) )
                {
                    PopupEntries::sliderFloat( std::nullopt, "Strength", &ui_state::world::world_tint_strength, 0.0f, 1.0f );
                    pop_tint.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Show Colliders##w_coll", &ui_state::maps::g_enabled,
            { (ImVec4*)ui_state::misc::g_misc_color_colliders } );

        CUIElements::toggle( ElemType::End, "Show Material##w_mat", &ui_state::misc::g_misc_show_material );

        if ( ui_state::world::world_fog_debug )
        {
            ImGui::Spacing( );
            ImGui::BeginChild( "fog_debug_logs", ImVec2( 0.f, 110.f ), true );
            ImGui::TextWrapped( "%s", ui_state::world::world_fog_log );
            ImGui::EndChild( );
        }
    }
}

// ─── World > Misc ────────────────────────────────────────────

void alice::tabs::callWorldMisc( int child )
{
    static CPopup pop_tp_binder( 160, "Teleport binds", std::nullopt );
    static CPopup pop_tp_settings( 170, "Bind settings", std::nullopt );
    static BindBool* tp_active_bind = nullptr;

    static CPopup pop_stop_binder( 160, "Stop binds", std::nullopt );
    static CPopup pop_stop_settings( 170, "Bind settings", std::nullopt );
    static BindBool* stop_active_bind = nullptr;

    if ( child == 1 )
    {
        CUIElements::toggle( ElemType::Begin, "Teleport##misc_tp", &ui_state::misc::g_misc_teleport_enabled, { },
            std::make_pair( &pop_tp_binder, []( ) {
                ctx->alice->binder->renderBool( &pop_tp_binder, &pop_tp_settings, "Teleport", &tp_active_bind,
                    ctx->config.teleportBinds, &ui_state::misc::g_misc_teleport_enabled );
            } ) );

        CUIElements::toggle( ElemType::End, "Stop##misc_stop", &ui_state::misc::g_misc_stop_enabled, { },
            std::make_pair( &pop_stop_binder, []( ) {
                ctx->alice->binder->renderBool( &pop_stop_binder, &pop_stop_settings, "Stop", &stop_active_bind,
                    ctx->config.stopBinds, &ui_state::misc::g_misc_stop_enabled );
            } ) );
    }
    else if ( child == 2 )
    {
        CUIElements::toggle( ElemType::Single, "Overlay##misc_ovl", &ui_state::world::b_obs_bypass );
    }
}

// ─── Legitbot > Aimbot ───────────────────────────────────────

void alice::tabs::callLegitbot( int child )
{
    static CPopup pop_aim_binder( 160, "Aimbot binds", std::nullopt );
    static CPopup pop_aim_settings( 170, "Bind settings", std::nullopt );
    static BindBool* aim_active_bind = nullptr;

    if ( child == 1 )
    {
        CUIElements::toggle( ElemType::Begin, "Aimbot##aim_en", &ui_state::entities::g_aimbot_enabled, { },
            std::make_pair( &pop_aim_binder, []( ) {
                if ( pop_aim_binder.begin( pop_aim_settings.opened ) )
                {
                    PopupEntries::sliderFloat( std::nullopt, "Radius", &ui_state::entities::g_aimbot_fov, 0.f, 180.f );
                    PopupEntries::separator( );

                    for ( int i = 0; i < (int)ctx->config.aimbotBinds.size( ); ++i )
                    {
                        auto& b = ctx->config.aimbotBinds[ i ];
                        ImGui::PushID( i );
                        b.alpha.update< float >( b.removed ? 0.f : 1.f );
                        ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle( ).Alpha * b.alpha.get< float >( ) );
                        PopupEntries::itemHeightOverride = ( ImGui::GetFontSize( ) + 14 ) * b.alpha.get< float >( );
                        if ( PopupEntries::button( ICON_FA_KEYBOARD, ( b.key == ImGuiKey_None ? "Unbound" : ImGui::GetKeyName( b.key ) ) ) && !pop_aim_settings.opened && pop_aim_settings.getAnimationValue( ) < 0.001f )
                        {
                            aim_active_bind = &b;
                            pop_aim_settings.opened = true;
                        }
                        PopupEntries::itemHeightOverride = std::nullopt;
                        ImGui::PopStyleVar( );
                        ImGui::PopID( );
                    }

                    if ( PopupEntries::button( ICON_FA_FLOPPY_DISK, "Add hotkey" ) )
                        ctx->config.aimbotBinds.push_back( BindBool { .type = BindType::Hold, .target = &ui_state::entities::g_aimbot_active, .name = "Aimbot" } );

                    pop_aim_binder.end( );
                }

                if ( pop_aim_settings.begin( ) )
                {
                    if ( aim_active_bind )
                    {
                        PopupEntries::bindListener( ICON_FA_KEYBOARD, "Key", &aim_active_bind->key, &aim_active_bind->listening );
                        PopupEntries::bindType( ICON_FA_TOGGLE_ON, "Type", &aim_active_bind->type );
                        PopupEntries::toggle( ICON_FA_EYE, "Visible", &aim_active_bind->visible );
                        PopupEntries::separator( );
                        PopupEntries::textColorOverride = ImColor( 191, 111, 105 );
                        if ( PopupEntries::button( ICON_FA_TRASH, "Remove" ) )
                        {
                            aim_active_bind->removed = true;
                            pop_aim_settings.opened = false;
                        }
                        PopupEntries::textColorOverride = std::nullopt;
                    }
                    pop_aim_settings.end( );
                }
            } ) );

        CUIElements::toggle( ElemType::Middle, "Show Fov##aim_fov_show", &ui_state::entities::g_aimbot_fov_display );

        static const std::vector< std::string > bones = { "Head", "Neck", "Body" };
        CUIElements::combo( ElemType::End, "Bones##aim_bones", &ui_state::entities::g_aimbot_target, bones );
    }
    else if ( child == 2 )
    {
    }
}

// ─── Legitbot > Triggerbot ───────────────────────────────────

void alice::tabs::callLegitbotTrigger( int child )
{
    static CPopup pop_trig_binder( 160, "Triggerbot binds", std::nullopt );
    static CPopup pop_trig_settings( 170, "Bind settings", std::nullopt );
    static BindBool* trig_active_bind = nullptr;

    if ( child == 1 )
    {
        CUIElements::toggle( ElemType::Begin, "Triggerbot##trig_en", &ui_state::trigger::g_trigger_enabled, { },
            std::make_pair( &pop_trig_binder, []( ) {
                if ( pop_trig_binder.begin( pop_trig_settings.opened ) )
                {
                    for ( int i = 0; i < (int)ctx->config.triggerbotBinds.size( ); ++i )
                    {
                        auto& b = ctx->config.triggerbotBinds[ i ];
                        ImGui::PushID( i );
                        b.alpha.update< float >( b.removed ? 0.f : 1.f );
                        ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle( ).Alpha * b.alpha.get< float >( ) );
                        PopupEntries::itemHeightOverride = ( ImGui::GetFontSize( ) + 14 ) * b.alpha.get< float >( );
                        if ( PopupEntries::button( ICON_FA_KEYBOARD, ( b.key == ImGuiKey_None ? "Unbound" : ImGui::GetKeyName( b.key ) ) ) && !pop_trig_settings.opened && pop_trig_settings.getAnimationValue( ) < 0.001f )
                        {
                            trig_active_bind = &b;
                            pop_trig_settings.opened = true;
                        }
                        PopupEntries::itemHeightOverride = std::nullopt;
                        ImGui::PopStyleVar( );
                        ImGui::PopID( );
                    }

                    if ( PopupEntries::button( ICON_FA_FLOPPY_DISK, "Add hotkey" ) )
                        ctx->config.triggerbotBinds.push_back( BindBool { .type = BindType::Hold, .target = &ui_state::trigger::g_trigger_active, .name = "Triggerbot" } );

                    pop_trig_binder.end( );
                }

                if ( pop_trig_settings.begin( ) )
                {
                    if ( trig_active_bind )
                    {
                        PopupEntries::bindListener( ICON_FA_KEYBOARD, "Key", &trig_active_bind->key, &trig_active_bind->listening );
                        PopupEntries::bindType( ICON_FA_TOGGLE_ON, "Type", &trig_active_bind->type );
                        PopupEntries::toggle( ICON_FA_EYE, "Visible", &trig_active_bind->visible );
                        PopupEntries::separator( );
                        PopupEntries::textColorOverride = ImColor( 191, 111, 105 );
                        if ( PopupEntries::button( ICON_FA_TRASH, "Remove" ) )
                        {
                            trig_active_bind->removed = true;
                            pop_trig_settings.opened = false;
                        }
                        PopupEntries::textColorOverride = std::nullopt;
                    }
                    pop_trig_settings.end( );
                }
            } ) );

        static std::vector< std::string > map_items;
        if ( (int)map_items.size( ) != ui_state::maps::g_collider_map_count )
        {
            map_items.clear( );
            for ( int i = 0; i < ui_state::maps::g_collider_map_count; ++i )
                map_items.push_back( ui_state::maps::g_collider_map_display_names[ i ] );
        }

        if ( !map_items.empty( ) )
        {
            // Label selection only; no map data is loaded.
            CUIElements::combo( ElemType::End, "Maps##trig_maps", &ui_state::maps::g_collider_map_index, map_items );
        }
    }
    else if ( child == 2 )
    {
    }
}

// ─── Stubs ───────────────────────────────────────────────────

void alice::tabs::callSkins( int child )
{
}
