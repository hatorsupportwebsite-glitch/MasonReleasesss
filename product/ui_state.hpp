#pragma once

// DATA ONLY: local menu values, with original defaults and option IDs.
// No game callbacks, addresses, process handles or external input generation.

namespace ui_state::entities
{
    inline bool g_aimbot_active = false;
    inline bool g_aimbot_enabled = false;
    inline float g_aimbot_fov = 90.f;
    inline bool g_aimbot_fov_display = true;
    inline int g_aimbot_key = 0x06;
    inline int g_aimbot_mode = 0;
    inline float g_aimbot_smooth = 0.5f;
    inline int g_aimbot_target = 0;
    inline bool g_esp_armor = false;
    inline bool g_esp_armor_flag_force = false;
    inline bool g_esp_armor_flag_glow = false;
    inline bool g_esp_armor_flag_glow_gradient = false;
    inline bool g_esp_armor_flag_gradient = true;
    inline bool g_esp_armor_flag_gradient_anim_left = false;
    inline bool g_esp_armor_flag_gradient_anim_right = false;
    inline bool g_esp_armor_glow = true;
    inline bool g_esp_armor_glow_gradient = false;
    inline bool g_esp_armor_gradient = true;
    inline bool g_esp_armor_show_text = false;
    inline bool g_esp_avatar = false;
    inline bool g_esp_box = true;
    inline bool g_esp_box_gradient_anim_left = false;
    inline bool g_esp_box_gradient_anim_right = false;
    inline bool g_esp_bullet_tracer = false;
    inline float g_esp_bullet_tracer_duration_sec = 1.5f;
    inline bool g_esp_chams_gradient_anim_left = false;
    inline bool g_esp_chams_gradient_anim_right = false;
    inline bool g_esp_circle = false;
    inline float g_esp_color_armor[4] = { 0.4f, 0.55f, 0.9f, 1.f };
    inline float g_esp_color_armor_flag[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_color_armor_flag_grad[4] = { 0.4f, 0.8f, 1.f, 1.f };
    inline float g_esp_color_armor_low[4] = { 0.2f, 0.35f, 0.7f, 1.f };
    inline float g_esp_color_arrows[4] = { 1.f, 1.f, 0.f, 1.f };
    inline float g_esp_color_box[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_color_box_grad[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_color_bullet_tracer[4] = { 1.f, 1.f, 1.f, 0.8f };
    inline float g_esp_color_circle[4] = { 1.f, 1.f, 1.f, 0.6f };
    inline float g_esp_color_device[4] = { 0.35f, 0.95f, 0.55f, 1.f };
    inline float g_esp_color_device_grad[4] = { 0.15f, 0.55f, 0.95f, 1.f };
    inline float g_esp_color_hit_marker[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_color_hit_marker_grad[4] = { 1.f, 0.2f, 0.2f, 1.f };
    inline float g_esp_color_hit_sparks[4] = { 0.20f, 0.95f, 1.00f, 1.f };
    inline float g_esp_color_hit_sparks_grad[4] = { 0.55f, 1.00f, 0.90f, 1.f };
    inline float g_esp_color_hitbox_fill[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_color_hitbox_fill_bottom[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_color_hp_full[4] = { 0.69f, 1.f, 0.31f, 1.f };
    inline float g_esp_color_hp_low[4] = { 1.f, 0.2f, 0.31f, 1.f };
    inline float g_esp_color_money[4] = { 0.4f, 1.f, 0.4f, 1.f };
    inline float g_esp_color_money_grad[4] = { 0.1f, 0.5f, 0.1f, 1.f };
    inline float g_esp_color_name[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_color_name_grad[4] = { 1.f, 0.6f, 0.2f, 1.f };
    inline float g_esp_color_ping[4] = { 0.6f, 0.8f, 1.f, 1.f };
    inline float g_esp_color_ping_grad[4] = { 0.2f, 0.5f, 1.f, 1.f };
    inline float g_esp_color_skeleton[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_color_snaplines[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_color_switching[4] = { 1.f, 0.85f, 0.35f, 1.f };
    inline float g_esp_color_switching_grad[4] = { 1.f, 0.45f, 0.15f, 1.f };
    inline float g_esp_color_weapon[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_color_weapon_grad[4] = { 1.f, 1.f, 1.f, 1.f };
    inline bool g_esp_corner_box = false;
    inline bool g_esp_device_enabled = false;
    inline bool g_esp_device_gradient_anim_left = false;
    inline bool g_esp_device_gradient_anim_right = false;
    inline bool g_esp_enabled = true;
    inline bool g_esp_filled = false;
    inline float g_esp_glow_color[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_glow_color_bottom[4] = { 0.6f, 0.8f, 1.f, 1.f };
    inline bool g_esp_glow_gradient_anim_left = false;
    inline bool g_esp_glow_gradient_anim_right = false;
    inline float g_esp_glow_radius = 30.f;
    inline float g_esp_grenade_icon_color[4] = { 1.f, 1.f, 1.f, 1.f };
    inline float g_esp_grenade_icon_color_grad[4] = { 0.45f, 0.9f, 1.f, 1.f };
    inline bool g_esp_grenade_particles = false;
    inline bool g_esp_grenade_show_radius = true;
    inline bool g_esp_grenade_show_timer = true;
    inline float g_esp_grenade_trail_color[4] = { 0.92f, 0.95f, 1.f, 1.f };
    inline float g_esp_grenade_trail_color_grad[4] = { 0.28f, 0.72f, 1.f, 1.f };
    inline float g_esp_grenade_trail_length_sec = 0.32f;
    inline bool g_esp_grenade_trail_particles = true;
    inline bool g_esp_grenade_trails = false;
    inline bool g_esp_grenades = false;
    inline bool g_esp_hit_marker = false;
    inline float g_esp_hit_marker_duration_sec = 1.5f;
    inline bool g_esp_hit_sparks = false;
    inline int g_esp_hit_sparks_count = 26;
    inline float g_esp_hit_sparks_duration_sec = 0.55f;
    inline float g_esp_hit_sparks_spread = 1.0f;
    inline bool g_esp_hitbox_glow = false;
    inline bool g_esp_hitbox_outline_fill = false;
    inline bool g_esp_hp = false;
    inline bool g_esp_hp_glow = true;
    inline bool g_esp_hp_glow_gradient = false;
    inline bool g_esp_hp_gradient = true;
    inline bool g_esp_hp_show_text = true;
    inline bool g_esp_kill_sound = false;
    inline int g_esp_kill_sound_index = 0;
    inline float g_esp_kill_sound_volume = 0.75f;
    inline float g_esp_molotov_circle_outline_color[4] = { 0.95f, 0.4f, 0.1f, 1.f };
    inline float g_esp_molotov_radius_m = 2.5f;
    inline bool g_esp_money = false;
    inline bool g_esp_money_glow = false;
    inline bool g_esp_money_glow_gradient = false;
    inline bool g_esp_money_gradient = true;
    inline bool g_esp_money_gradient_anim_left = false;
    inline bool g_esp_money_gradient_anim_right = false;
    inline bool g_esp_name = false;
    inline bool g_esp_name_glow = false;
    inline bool g_esp_name_glow_gradient = false;
    inline bool g_esp_name_gradient = true;
    inline bool g_esp_name_gradient_anim_left = false;
    inline bool g_esp_name_gradient_anim_right = false;
    inline bool g_esp_oof_arrows = false;
    inline bool g_esp_ping = false;
    inline bool g_esp_ping_glow = false;
    inline bool g_esp_ping_glow_gradient = false;
    inline bool g_esp_ping_gradient = true;
    inline bool g_esp_ping_gradient_anim_left = false;
    inline bool g_esp_ping_gradient_anim_right = false;
    inline bool g_esp_skeleton = false;
    inline float g_esp_smoke_circle_outline_color[4] = { 0.5f, 0.25f, 0.95f, 1.f };
    inline float g_esp_smoke_radius_m = 3.f;
    inline bool g_esp_smooth_elements = false;
    inline bool g_esp_snaplines_bottom = false;
    inline bool g_esp_snaplines_top = false;
    inline bool g_esp_switching = false;
    inline bool g_esp_switching_gradient = true;
    inline bool g_esp_switching_gradient_anim_left = false;
    inline bool g_esp_switching_gradient_anim_right = false;
    inline bool g_esp_weapon = false;
    inline bool g_esp_weapon_glow = false;
    inline bool g_esp_weapon_glow_gradient = false;
    inline bool g_esp_weapon_gradient_anim_left = false;
    inline bool g_esp_weapon_gradient_anim_right = false;
    inline bool g_esp_weapon_icon = true;
    inline bool g_esp_weapon_text = true;
    inline bool g_kill_animated = false;
    inline float g_kill_fade_duration_sec = 1.5f;
    inline bool g_soul_animation = false;
    inline float g_soul_appear_delay_sec = 0.f;
    inline float g_soul_color_bottom[4] = { 1.f, 0.25f, 0.65f, 1.f };
    inline float g_soul_color_top[4] = { 1.f, 0.55f, 0.2f, 1.f };
    inline bool g_soul_gradient = true;
    inline bool g_soul_gradient_anim_left = false;
    inline bool g_soul_gradient_anim_right = false;
    inline float g_soul_lifetime_sec = 2.f;
}

namespace ui_state::misc
{
    inline float g_misc_color_colliders[4] = { 0.4f, 1.f, 0.7f, 1.f };
    inline bool g_misc_keybinds = true;
    inline bool g_misc_show_material = false;
    inline bool g_misc_stop_enabled = false;
    inline int g_misc_stop_key = 0;
    inline bool g_misc_teleport_enabled = false;
    inline int g_misc_teleport_key = 0;
    inline bool g_misc_watermark = false;
}

namespace ui_state::trigger
{
    inline bool g_trigger_active = false;
    inline bool g_trigger_enabled = false;
    inline int g_trigger_key_vk = 0x01;
    inline int g_trigger_mode = 0;
}

namespace ui_state::extra
{
    inline bool g_silent_active = false;
    inline bool g_silent_autofire = false;
    inline bool g_silent_enabled = false;
    inline float g_silent_fov = 90.f;
    inline float g_silent_fov_color[4] = { 1.f, 0.4f, 0.4f, 0.7f };
    inline bool g_silent_fov_display = true;
    inline int g_silent_key = 0x05;
    inline int g_silent_mode = 0;
    inline int g_silent_target_bone = 0;
}

namespace ui_state::world
{
    inline bool b_obs_bypass = false;
    inline bool night_mode = false;
    inline float world_fog_color[4] = { 0.58f, 0.78f, 0.95f, 1.0f };
    inline bool world_fog_debug = false;
    inline float world_fog_density = 0.028f;
    inline bool world_fog_enable = false;
    inline float world_fog_end = 72.0f;
    inline char world_fog_log[2048] = "Fog debug log is empty.";
    inline float world_fog_start = 14.0f;
    inline float world_sky_color[4] = { 0.42f, 0.66f, 0.95f, 1.0f };
    inline bool world_sky_enable = false;
    inline float world_sky_intensity = 1.2f;
    inline float world_tint_color[4] = { 0.92f, 0.95f, 1.0f, 1.0f };
    inline bool world_tint_enable = false;
    inline float world_tint_strength = 0.25f;
}

namespace ui_state::maps
{
    inline bool g_enabled = false;
    inline int g_collider_map_index = 0;
    inline constexpr const char* g_collider_map_display_names[] = {
	"sandstone", "province", "dune", "rust", "zone7", "breeze", "hanami",
	"arena", "cobleway", "favelas", "hanari", "pipeline", "bridge", "yard"
    };
    inline constexpr int g_collider_map_count = static_cast<int>(sizeof(g_collider_map_display_names)/sizeof(g_collider_map_display_names[0]));
}
