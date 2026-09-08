#include "configs.h"
#include "ui_state.hpp"
#include "backend/imgui/imgui.h"
#include "everness/src/alice/ui/themes/themes.hpp"
#include "everness/src/core.hpp"
#include "everness/src/config/data_management.hpp"
#include "everness/src/alice/ui/widgets/widgets.hpp"
#include "everness/src/alice/ui/widgets/watermark/watermark.hpp"
#include "everness/src/alice/ui/widgets/keybinds/keybinds.hpp"
#include "everness/src/alice/ui/widgets/hit_logs/hit_logs.hpp"
#include "everness/src/alice/ui/binder/binder.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <shellapi.h>
#pragma comment(lib, "advapi32.lib")
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <vector>

#pragma comment(lib, "shell32.lib")

namespace configs
{
	static thread_local char g_err[256];

	const char* last_error()
	{
		return g_err;
	}

	static void set_err(const char* s)
	{
		snprintf(g_err, sizeof(g_err), "%s", s ? s : "");
	}

	std::wstring get_config_dir_w()
	{
		wchar_t path[MAX_PATH] = {};
		if (FAILED(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, path)))
			return L"";
		std::wstring dir(path);
		if (!dir.empty() && dir.back() != L'\\') dir += L'\\';
		dir += L"MasonRecode";
		return dir;
	}

	std::string get_config_dir()
	{
		std::wstring w = get_config_dir_w();
		if (w.empty()) return {};
		int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (n <= 0) return {};
		std::string s((size_t)n, '\0');
		WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), n, nullptr, nullptr);
		s.pop_back();
		return s;
	}

	std::wstring get_killsound_dir_w()
	{
		std::wstring dir = get_config_dir_w();
		if (dir.empty()) return L"";
		if (dir.back() != L'\\' && dir.back() != L'/') dir += L'\\';
		dir += L"killsound";
		return dir;
	}

	bool ensure_directory()
	{
		std::wstring dir = get_config_dir_w();
		if (dir.empty()) { set_err("Documents path"); return false; }
		if (CreateDirectoryW(dir.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS)
			return true;
		set_err("CreateDirectory");
		return false;
	}

	std::string sanitize_config_name(const char* name)
	{
		if (!name) return {};
		std::string o;
		for (const char* p = name; *p; ++p) {
			unsigned char c = (unsigned char)*p;
			if (std::isalnum(c) || c == '_' || c == '-' || c == ' ')
				o += (char)c;
		}
		while (!o.empty() && o.front() == ' ') o.erase(o.begin());
		while (!o.empty() && o.back() == ' ') o.pop_back();
		for (char& c : o) if (c == ' ') c = '_';
		if (o.size() > 64) o.resize(64);
		return o;
	}

	static const uint8_t k_magic[8] = { 'M', 'R', 'C', 'F', 'G', 1, 0, 0 };

	static void xor_crypt(uint8_t* data, size_t len, const uint8_t salt[16])
	{
		static const uint8_t k_seed[] =
			"MasonRecode_CfgKey_v1_\x7f\x91\xaa";
		for (size_t i = 0; i < len; ++i) {
			uint8_t k = (uint8_t)(k_seed[i % (sizeof(k_seed) - 1)] ^ salt[i % 16] ^ (uint8_t)(i * 1103515245u));
			data[i] ^= k;
		}
	}

	static void f4(std::ostringstream& o, const char* k, const float v[4])
	{
		char b[128];
		snprintf(b, sizeof(b), "%s=%.8g,%.8g,%.8g,%.8g\n", k, v[0], v[1], v[2], v[3]);
		o << b;
	}

	static void bf(std::ostringstream& o, const char* k, bool v) { o << k << '=' << (v ? 1 : 0) << '\n'; }
	static void sf(std::ostringstream& o, const char* k, float v)
	{
		char b[64]; snprintf(b, sizeof(b), "%s=%.8g\n", k, v); o << b;
	}
	static void intf(std::ostringstream& o, const char* k, int v) { o << k << '=' << v << '\n'; }

	static void clear_config_binds()
	{
		if (!everness::ctx)
			return;
		everness::Config& c = everness::ctx->config;
		c.espRectBinds.clear();
		c.teleportBinds.clear();
		c.stopBinds.clear();
		c.aimbotBinds.clear();
		c.triggerbotBinds.clear();
		c.autofireBinds.clear();
	}

	static void append_bind_lines(std::ostringstream& o)
	{
		if (!everness::ctx || !everness::ctx->alice)
			return;
		const everness::Config& c = everness::ctx->config;
		auto emit_group = [&](const char* group, const std::vector<alice::BindBool>& v) {
			for (const auto& b : v) {
				if (b.removed)
					continue;
				const int ti = (b.type == alice::BindType::Toggle) ? 0 : 1;
				const int vis = b.visible ? 1 : 0;
				char buf[192];
				snprintf(buf, sizeof(buf), "bb=%s|%d|%d|%d\n", group, ti, (int)b.key, vis);
				o << buf;
			}
		};
		emit_group("esp_rect", c.espRectBinds);
		emit_group("teleport", c.teleportBinds);
		emit_group("stop", c.stopBinds);
		emit_group("aimbot", c.aimbotBinds);
		emit_group("triggerbot", c.triggerbotBinds);
		emit_group("silent", c.autofireBinds);
	}

	static void apply_bind_line(const std::string& val)
	{
		if (!everness::ctx || !everness::ctx->alice)
			return;
		const size_t p0 = val.find('|');
		const size_t p1 = val.find('|', p0 + 1);
		const size_t p2 = val.find('|', p1 + 1);
		if (p0 == std::string::npos || p1 == std::string::npos || p2 == std::string::npos)
			return;
		const std::string group = val.substr(0, p0);
		const int ti = atoi(val.substr(p0 + 1, p1 - p0 - 1).c_str());
		const int ki = atoi(val.substr(p1 + 1, p2 - p1 - 1).c_str());
		const int vis = atoi(val.substr(p2 + 1).c_str());

		alice::BindBool b{};
		b.type = (ti == 0) ? alice::BindType::Toggle : alice::BindType::Hold;
		b.key = (ImGuiKey)ki;
		b.visible = (vis != 0);
		b.listening = false;
		b.active = false;
		b.removed = false;
		b.ownsTarget = false;

		everness::Config& c = everness::ctx->config;
		if (group == "esp_rect") {
			b.target = &everness::c::get<bool>(c.espRect);
			b.name = "Rect";
			c.espRectBinds.push_back(b);
		} else if (group == "teleport") {
			b.target = &ui_state::misc::g_misc_teleport_enabled;
			b.name = "Teleport";
			c.teleportBinds.push_back(b);
		} else if (group == "stop") {
			b.target = &ui_state::misc::g_misc_stop_enabled;
			b.name = "Stop";
			c.stopBinds.push_back(b);
		} else if (group == "aimbot") {
			b.target = &ui_state::entities::g_aimbot_active;
			b.name = "Aimbot";
			c.aimbotBinds.push_back(b);
		} else if (group == "triggerbot") {
			b.target = &ui_state::trigger::g_trigger_active;
			b.name = "Triggerbot";
			c.triggerbotBinds.push_back(b);
		} else if (group == "silent") {
			b.target = &ui_state::extra::g_silent_active;
			b.name = "Silent";
			c.autofireBinds.push_back(b);
		}
	}

	static std::string build_plaintext()
	{
		std::ostringstream o;
		o << "v=2\n";
		bf(o, "esp_enabled", ui_state::entities::g_esp_enabled);
		bf(o, "g_esp_box", ui_state::entities::g_esp_box);
		bf(o, "g_esp_box_gradient_anim_left", ui_state::entities::g_esp_box_gradient_anim_left);
		bf(o, "g_esp_box_gradient_anim_right", ui_state::entities::g_esp_box_gradient_anim_right);
		bf(o, "g_esp_hp", ui_state::entities::g_esp_hp);
		bf(o, "g_esp_name", ui_state::entities::g_esp_name);
		bf(o, "g_esp_skeleton", ui_state::entities::g_esp_skeleton);
		bf(o, "g_esp_hitbox_outline_fill", ui_state::entities::g_esp_hitbox_outline_fill);
		bf(o, "g_esp_chams_gradient_anim_left", ui_state::entities::g_esp_chams_gradient_anim_left);
		bf(o, "g_esp_chams_gradient_anim_right", ui_state::entities::g_esp_chams_gradient_anim_right);
		bf(o, "g_esp_hitbox_glow", ui_state::entities::g_esp_hitbox_glow);
		bf(o, "g_esp_glow_gradient_anim_left", ui_state::entities::g_esp_glow_gradient_anim_left);
		bf(o, "g_esp_glow_gradient_anim_right", ui_state::entities::g_esp_glow_gradient_anim_right);
		bf(o, "g_esp_weapon", ui_state::entities::g_esp_weapon);
		bf(o, "g_esp_armor", ui_state::entities::g_esp_armor);
		bf(o, "g_esp_avatar", ui_state::entities::g_esp_avatar);
		bf(o, "g_esp_grenades", ui_state::entities::g_esp_grenades);
		bf(o, "g_esp_grenade_show_timer", ui_state::entities::g_esp_grenade_show_timer);
		bf(o, "g_esp_grenade_show_radius", ui_state::entities::g_esp_grenade_show_radius);
		bf(o, "g_esp_grenade_particles", ui_state::entities::g_esp_grenade_particles);
		bf(o, "g_esp_grenade_trails", ui_state::entities::g_esp_grenade_trails);
		bf(o, "g_esp_grenade_trail_particles", ui_state::entities::g_esp_grenade_trail_particles);
		sf(o, "g_esp_grenade_trail_length_sec", ui_state::entities::g_esp_grenade_trail_length_sec);
		f4(o, "g_esp_grenade_trail_color", ui_state::entities::g_esp_grenade_trail_color);
		f4(o, "g_esp_grenade_trail_color_grad", ui_state::entities::g_esp_grenade_trail_color_grad);
		bf(o, "g_esp_hp_gradient", ui_state::entities::g_esp_hp_gradient);
		bf(o, "g_esp_hp_show_text", ui_state::entities::g_esp_hp_show_text);
		bf(o, "g_esp_hp_glow", ui_state::entities::g_esp_hp_glow);
		bf(o, "g_esp_hp_glow_gradient", ui_state::entities::g_esp_hp_glow_gradient);
		bf(o, "g_esp_armor_gradient", ui_state::entities::g_esp_armor_gradient);
		bf(o, "g_esp_armor_show_text", ui_state::entities::g_esp_armor_show_text);
		bf(o, "g_esp_armor_glow", ui_state::entities::g_esp_armor_glow);
		bf(o, "g_esp_armor_glow_gradient", ui_state::entities::g_esp_armor_glow_gradient);
		bf(o, "g_esp_corner_box", ui_state::entities::g_esp_corner_box);
		bf(o, "g_esp_filled", ui_state::entities::g_esp_filled);
		bf(o, "g_esp_snaplines_top", ui_state::entities::g_esp_snaplines_top);
		bf(o, "g_esp_snaplines_bottom", ui_state::entities::g_esp_snaplines_bottom);
		bf(o, "g_esp_circle", ui_state::entities::g_esp_circle);
		bf(o, "g_esp_oof_arrows", ui_state::entities::g_esp_oof_arrows);
		bf(o, "g_esp_money", ui_state::entities::g_esp_money);
		bf(o, "g_esp_ping", ui_state::entities::g_esp_ping);
		bf(o, "g_esp_name_gradient", ui_state::entities::g_esp_name_gradient);
		bf(o, "g_esp_name_gradient_anim_left", ui_state::entities::g_esp_name_gradient_anim_left);
		bf(o, "g_esp_name_gradient_anim_right", ui_state::entities::g_esp_name_gradient_anim_right);
		bf(o, "g_esp_weapon_gradient_anim_left", ui_state::entities::g_esp_weapon_gradient_anim_left);
		bf(o, "g_esp_weapon_gradient_anim_right", ui_state::entities::g_esp_weapon_gradient_anim_right);
		bf(o, "g_esp_weapon_glow", ui_state::entities::g_esp_weapon_glow);
		bf(o, "g_esp_weapon_glow_gradient", ui_state::entities::g_esp_weapon_glow_gradient);
		bf(o, "g_esp_name_glow", ui_state::entities::g_esp_name_glow);
		bf(o, "g_esp_name_glow_gradient", ui_state::entities::g_esp_name_glow_gradient);
		bf(o, "g_esp_money_glow", ui_state::entities::g_esp_money_glow);
		bf(o, "g_esp_money_glow_gradient", ui_state::entities::g_esp_money_glow_gradient);
		bf(o, "g_esp_ping_glow", ui_state::entities::g_esp_ping_glow);
		bf(o, "g_esp_ping_glow_gradient", ui_state::entities::g_esp_ping_glow_gradient);
		bf(o, "g_esp_money_gradient", ui_state::entities::g_esp_money_gradient);
		bf(o, "g_esp_money_gradient_anim_left", ui_state::entities::g_esp_money_gradient_anim_left);
		bf(o, "g_esp_money_gradient_anim_right", ui_state::entities::g_esp_money_gradient_anim_right);
		bf(o, "g_esp_ping_gradient", ui_state::entities::g_esp_ping_gradient);
		bf(o, "g_esp_ping_gradient_anim_left", ui_state::entities::g_esp_ping_gradient_anim_left);
		bf(o, "g_esp_ping_gradient_anim_right", ui_state::entities::g_esp_ping_gradient_anim_right);
		bf(o, "g_esp_bullet_tracer", ui_state::entities::g_esp_bullet_tracer);
		bf(o, "g_esp_hit_marker", ui_state::entities::g_esp_hit_marker);
		bf(o, "g_esp_hit_sparks", ui_state::entities::g_esp_hit_sparks);
		bf(o, "g_esp_kill_sound", ui_state::entities::g_esp_kill_sound);
		sf(o, "g_esp_kill_sound_volume", ui_state::entities::g_esp_kill_sound_volume);
		intf(o, "g_esp_kill_sound_index", ui_state::entities::g_esp_kill_sound_index);
		sf(o, "g_esp_bullet_tracer_duration_sec", ui_state::entities::g_esp_bullet_tracer_duration_sec);
		sf(o, "g_esp_hit_marker_duration_sec", ui_state::entities::g_esp_hit_marker_duration_sec);
		sf(o, "g_esp_hit_sparks_duration_sec", ui_state::entities::g_esp_hit_sparks_duration_sec);
		intf(o, "g_esp_hit_sparks_count", ui_state::entities::g_esp_hit_sparks_count);
		sf(o, "g_esp_hit_sparks_spread", ui_state::entities::g_esp_hit_sparks_spread);
		bf(o, "g_esp_armor_flag_gradient", ui_state::entities::g_esp_armor_flag_gradient);
		bf(o, "g_esp_armor_flag_gradient_anim_left", ui_state::entities::g_esp_armor_flag_gradient_anim_left);
		bf(o, "g_esp_armor_flag_gradient_anim_right", ui_state::entities::g_esp_armor_flag_gradient_anim_right);
		bf(o, "g_esp_armor_flag_glow", ui_state::entities::g_esp_armor_flag_glow);
		bf(o, "g_esp_armor_flag_glow_gradient", ui_state::entities::g_esp_armor_flag_glow_gradient);
		bf(o, "g_esp_armor_flag_force", ui_state::entities::g_esp_armor_flag_force);
		bf(o, "g_esp_device_enabled", ui_state::entities::g_esp_device_enabled);
		bf(o, "g_esp_device_gradient_anim_left", ui_state::entities::g_esp_device_gradient_anim_left);
		bf(o, "g_esp_device_gradient_anim_right", ui_state::entities::g_esp_device_gradient_anim_right);
		bf(o, "g_esp_switching", ui_state::entities::g_esp_switching);
		bf(o, "g_esp_switching_gradient", ui_state::entities::g_esp_switching_gradient);
		bf(o, "g_esp_switching_gradient_anim_left", ui_state::entities::g_esp_switching_gradient_anim_left);
		bf(o, "g_esp_switching_gradient_anim_right", ui_state::entities::g_esp_switching_gradient_anim_right);
		bf(o, "g_kill_animated", ui_state::entities::g_kill_animated);
		sf(o, "g_kill_fade_duration_sec", ui_state::entities::g_kill_fade_duration_sec);
		bf(o, "g_soul_animation", ui_state::entities::g_soul_animation);
		bf(o, "g_soul_gradient", ui_state::entities::g_soul_gradient);
		bf(o, "g_soul_gradient_anim_left", ui_state::entities::g_soul_gradient_anim_left);
		bf(o, "g_soul_gradient_anim_right", ui_state::entities::g_soul_gradient_anim_right);
		sf(o, "g_soul_appear_delay_sec", ui_state::entities::g_soul_appear_delay_sec);
		sf(o, "g_soul_lifetime_sec", ui_state::entities::g_soul_lifetime_sec);
		bf(o, "g_esp_smooth_elements", ui_state::entities::g_esp_smooth_elements);
		sf(o, "g_esp_smoke_radius_m", ui_state::entities::g_esp_smoke_radius_m);
		sf(o, "g_esp_molotov_radius_m", ui_state::entities::g_esp_molotov_radius_m);
		sf(o, "g_esp_glow_radius", ui_state::entities::g_esp_glow_radius);
		bf(o, "g_aimbot_enabled", ui_state::entities::g_aimbot_enabled);
		sf(o, "g_aimbot_fov", ui_state::entities::g_aimbot_fov);
		bf(o, "g_aimbot_fov_display", ui_state::entities::g_aimbot_fov_display);
		intf(o, "g_aimbot_key", ui_state::entities::g_aimbot_key);
		intf(o, "g_aimbot_mode", ui_state::entities::g_aimbot_mode);
		intf(o, "g_aimbot_target", ui_state::entities::g_aimbot_target);
		bf(o, "g_silent_enabled", ui_state::extra::g_silent_enabled);
		bf(o, "g_silent_autofire", ui_state::extra::g_silent_autofire);
		sf(o, "g_silent_fov", ui_state::extra::g_silent_fov);
		intf(o, "g_silent_target_bone", ui_state::extra::g_silent_target_bone);
		bf(o, "g_silent_fov_display", ui_state::extra::g_silent_fov_display);
		intf(o, "g_silent_key", ui_state::extra::g_silent_key);
		intf(o, "g_silent_mode", ui_state::extra::g_silent_mode);
		sf(o, "g_aimbot_smooth", ui_state::entities::g_aimbot_smooth);
		bf(o, "g_trigger_enabled", ui_state::trigger::g_trigger_enabled);
		intf(o, "g_trigger_key_vk", ui_state::trigger::g_trigger_key_vk);
		intf(o, "g_trigger_mode", ui_state::trigger::g_trigger_mode);
		bf(o, "g_misc_stop_enabled", ui_state::misc::g_misc_stop_enabled);
		intf(o, "g_misc_stop_key", ui_state::misc::g_misc_stop_key);
		bf(o, "g_misc_teleport_enabled", ui_state::misc::g_misc_teleport_enabled);
		intf(o, "g_misc_teleport_key", ui_state::misc::g_misc_teleport_key);
		bf(o, "g_misc_show_material", ui_state::misc::g_misc_show_material);
		bf(o, "g_misc_watermark", ui_state::misc::g_misc_watermark);
		bf(o, "g_misc_keybinds", ui_state::misc::g_misc_keybinds);
		bf(o, "Drawing_b_obs_bypass", ui_state::world::b_obs_bypass);
		bf(o, "Drawing_night_mode", ui_state::world::night_mode);
		bf(o, "Drawing_world_fog_enable", ui_state::world::world_fog_enable);
		bf(o, "Drawing_world_fog_debug", ui_state::world::world_fog_debug);
		bf(o, "Drawing_world_sky_enable", ui_state::world::world_sky_enable);
		bf(o, "Drawing_world_tint_enable", ui_state::world::world_tint_enable);
		sf(o, "Drawing_world_fog_start", ui_state::world::world_fog_start);
		sf(o, "Drawing_world_fog_end", ui_state::world::world_fog_end);
		sf(o, "Drawing_world_fog_density", ui_state::world::world_fog_density);
		sf(o, "Drawing_world_sky_intensity", ui_state::world::world_sky_intensity);
		sf(o, "Drawing_world_tint_strength", ui_state::world::world_tint_strength);
		bf(o, "map_g_enabled", ui_state::maps::g_enabled);
		intf(o, "map_g_collider_map_index", ui_state::maps::g_collider_map_index);
		f4(o, "accent", &alice::CThemes::current.colors.accent.x);
		f4(o, "Drawing_world_fog_color", ui_state::world::world_fog_color);
		f4(o, "Drawing_world_sky_color", ui_state::world::world_sky_color);
		f4(o, "Drawing_world_tint_color", ui_state::world::world_tint_color);
		f4(o, "g_esp_color_box", ui_state::entities::g_esp_color_box);
		f4(o, "g_esp_color_box_grad", ui_state::entities::g_esp_color_box_grad);
		f4(o, "g_esp_color_name", ui_state::entities::g_esp_color_name);
		f4(o, "g_esp_color_hp_full", ui_state::entities::g_esp_color_hp_full);
		f4(o, "g_esp_color_hp_low", ui_state::entities::g_esp_color_hp_low);
		f4(o, "g_esp_color_armor", ui_state::entities::g_esp_color_armor);
		f4(o, "g_esp_color_armor_low", ui_state::entities::g_esp_color_armor_low);
		f4(o, "g_esp_color_weapon", ui_state::entities::g_esp_color_weapon);
		f4(o, "g_esp_color_weapon_grad", ui_state::entities::g_esp_color_weapon_grad);
		f4(o, "g_esp_color_skeleton", ui_state::entities::g_esp_color_skeleton);
		f4(o, "g_esp_color_hitbox_fill", ui_state::entities::g_esp_color_hitbox_fill);
		f4(o, "g_esp_color_hitbox_fill_bottom", ui_state::entities::g_esp_color_hitbox_fill_bottom);
		f4(o, "g_esp_glow_color", ui_state::entities::g_esp_glow_color);
		f4(o, "g_esp_glow_color_bottom", ui_state::entities::g_esp_glow_color_bottom);
		f4(o, "g_esp_smoke_circle_outline_color", ui_state::entities::g_esp_smoke_circle_outline_color);
		f4(o, "g_esp_molotov_circle_outline_color", ui_state::entities::g_esp_molotov_circle_outline_color);
		f4(o, "g_esp_color_snaplines", ui_state::entities::g_esp_color_snaplines);
		f4(o, "g_esp_color_circle", ui_state::entities::g_esp_color_circle);
		f4(o, "g_esp_color_arrows", ui_state::entities::g_esp_color_arrows);
		f4(o, "g_esp_color_money", ui_state::entities::g_esp_color_money);
		f4(o, "g_esp_color_ping", ui_state::entities::g_esp_color_ping);
		f4(o, "g_esp_color_money_grad", ui_state::entities::g_esp_color_money_grad);
		f4(o, "g_esp_color_ping_grad", ui_state::entities::g_esp_color_ping_grad);
		f4(o, "g_esp_color_device", ui_state::entities::g_esp_color_device);
		f4(o, "g_esp_color_device_grad", ui_state::entities::g_esp_color_device_grad);
		f4(o, "g_esp_color_switching", ui_state::entities::g_esp_color_switching);
		f4(o, "g_esp_color_switching_grad", ui_state::entities::g_esp_color_switching_grad);
		f4(o, "g_esp_color_name_grad", ui_state::entities::g_esp_color_name_grad);
		f4(o, "g_esp_color_hit_marker", ui_state::entities::g_esp_color_hit_marker);
		f4(o, "g_esp_color_hit_marker_grad", ui_state::entities::g_esp_color_hit_marker_grad);
		f4(o, "g_esp_color_hit_sparks", ui_state::entities::g_esp_color_hit_sparks);
		f4(o, "g_esp_color_hit_sparks_grad", ui_state::entities::g_esp_color_hit_sparks_grad);
		f4(o, "g_soul_color_top", ui_state::entities::g_soul_color_top);
		f4(o, "g_soul_color_bottom", ui_state::entities::g_soul_color_bottom);
		f4(o, "g_esp_color_armor_flag", ui_state::entities::g_esp_color_armor_flag);
		f4(o, "g_esp_color_armor_flag_grad", ui_state::entities::g_esp_color_armor_flag_grad);
		f4(o, "g_esp_color_bullet_tracer", ui_state::entities::g_esp_color_bullet_tracer);
		f4(o, "g_esp_grenade_icon_color", ui_state::entities::g_esp_grenade_icon_color);
		f4(o, "g_esp_grenade_icon_color_grad", ui_state::entities::g_esp_grenade_icon_color_grad);
		f4(o, "g_silent_fov_color", ui_state::extra::g_silent_fov_color);
		if (everness::ctx && everness::ctx->alice && everness::ctx->alice->widgets) {
			alice::CWidgets& w = *everness::ctx->alice->widgets;
			if (w.watermark)
				bf(o, "ui_widget_watermark", w.watermark->enabled);
			if (w.keybinds)
				bf(o, "ui_widget_keybinds", w.keybinds->enabled);
			if (w.hitLogs)
				bf(o, "ui_widget_hitlogs", w.hitLogs->enabled);
		}
		append_bind_lines(o);
		return o.str();
	}

	static void parse_f4(const std::string& v, float out[4])
	{
		float a, b, c, d;
		if (sscanf_s(v.c_str(), "%f,%f,%f,%f", &a, &b, &c, &d) == 4) {
			out[0] = a; out[1] = b; out[2] = c; out[3] = d;
		}
	}

	static void apply_line(const std::string& key, const std::string& val)
	{
		// Keep this branch shallow: configs.cpp already has a very long else-if chain.
		if (key == "Drawing_world_sky_enable")
		{
			ui_state::world::world_sky_enable = (val == "1");
			return;
		}
		if (key == "Drawing_world_sky_intensity")
		{
			ui_state::world::world_sky_intensity = (float)atof(val.c_str());
			return;
		}
		if (key == "Drawing_world_sky_color")
		{
			parse_f4(val, ui_state::world::world_sky_color);
			return;
		}
		if (key == "Drawing_world_tint_enable")
		{
			ui_state::world::world_tint_enable = (val == "1");
			return;
		}
		if (key == "Drawing_world_tint_strength")
		{
			ui_state::world::world_tint_strength = (float)atof(val.c_str());
			return;
		}
		if (key == "Drawing_world_tint_color")
		{
			parse_f4(val, ui_state::world::world_tint_color);
			return;
		}
		// Keep the main else-if chain under MSVC nesting limits.
		if (key == "g_esp_hit_marker")
		{
			ui_state::entities::g_esp_hit_marker = (val == "1");
			return;
		}
		if (key == "g_esp_kill_sound") { ui_state::entities::g_esp_kill_sound = (val == "1"); return; }
		if (key == "g_esp_kill_sound_volume") { ui_state::entities::g_esp_kill_sound_volume = (float)atof(val.c_str()); return; }
		if (key == "g_esp_kill_sound_index") { ui_state::entities::g_esp_kill_sound_index = atoi(val.c_str()); return; }
		if (key == "g_esp_bullet_tracer_duration_sec")
		{
			ui_state::entities::g_esp_bullet_tracer_duration_sec = (float)atof(val.c_str());
			return;
		}
		if (key == "g_esp_hit_marker_duration_sec")
		{
			ui_state::entities::g_esp_hit_marker_duration_sec = (float)atof(val.c_str());
			return;
		}
		if (key == "g_esp_hit_sparks")
		{
			ui_state::entities::g_esp_hit_sparks = (val == "1");
			return;
		}
		if (key == "g_esp_hit_sparks_duration_sec")
		{
			ui_state::entities::g_esp_hit_sparks_duration_sec = (float)atof(val.c_str());
			return;
		}
		if (key == "g_esp_hit_sparks_count")
		{
			ui_state::entities::g_esp_hit_sparks_count = atoi(val.c_str());
			return;
		}
		if (key == "g_esp_hit_sparks_spread")
		{
			ui_state::entities::g_esp_hit_sparks_spread = (float)atof(val.c_str());
			return;
		}
		if (key == "g_esp_color_hit_marker")
		{
			parse_f4(val, ui_state::entities::g_esp_color_hit_marker);
			return;
		}
		if (key == "g_esp_color_hit_marker_grad")
		{
			parse_f4(val, ui_state::entities::g_esp_color_hit_marker_grad);
			return;
		}
		if (key == "g_esp_color_hit_sparks")
		{
			parse_f4(val, ui_state::entities::g_esp_color_hit_sparks);
			return;
		}
		if (key == "g_esp_color_hit_sparks_grad")
		{
			parse_f4(val, ui_state::entities::g_esp_color_hit_sparks_grad);
			return;
		}
		if (key == "g_soul_color_top")
		{
			parse_f4(val, ui_state::entities::g_soul_color_top);
			return;
		}
		if (key == "g_soul_color_bottom")
		{
			parse_f4(val, ui_state::entities::g_soul_color_bottom);
			return;
		}
		if (key == "g_esp_bullet_tracer")
		{
			ui_state::entities::g_esp_bullet_tracer = (val == "1");
			return;
		}

		// Flat if+return: MSVC C1061 on deep else-if chains in this function.
		if (key == "esp_enabled") { ui_state::entities::g_esp_enabled = (val == "1"); return; }
		if (key == "g_esp_box") { ui_state::entities::g_esp_box = (val == "1"); return; }
		if (key == "g_esp_box_gradient_anim_left") { ui_state::entities::g_esp_box_gradient_anim_left = (val == "1"); return; }
		if (key == "g_esp_box_gradient_anim_right") { ui_state::entities::g_esp_box_gradient_anim_right = (val == "1"); return; }
		if (key == "g_esp_box_gradient_animated") { ui_state::entities::g_esp_box_gradient_anim_left = (val == "1"); ui_state::entities::g_esp_box_gradient_anim_right = false; return; }
		if (key == "g_esp_hp") { ui_state::entities::g_esp_hp = (val == "1"); return; }
		if (key == "g_esp_name") { ui_state::entities::g_esp_name = (val == "1"); return; }
		if (key == "g_esp_skeleton") { ui_state::entities::g_esp_skeleton = (val == "1"); return; }
		if (key == "g_esp_hitbox_outline_fill") { ui_state::entities::g_esp_hitbox_outline_fill = (val == "1"); return; }
		if (key == "g_esp_chams_gradient_anim_left") { ui_state::entities::g_esp_chams_gradient_anim_left = (val == "1"); return; }
		if (key == "g_esp_chams_gradient_anim_right") { ui_state::entities::g_esp_chams_gradient_anim_right = (val == "1"); return; }
		if (key == "g_esp_chams_gradient_animated") { ui_state::entities::g_esp_chams_gradient_anim_left = (val == "1"); ui_state::entities::g_esp_chams_gradient_anim_right = false; return; }
		if (key == "g_esp_hitbox_glow") { ui_state::entities::g_esp_hitbox_glow = (val == "1"); return; }
		if (key == "g_esp_glow_gradient_anim_left") { ui_state::entities::g_esp_glow_gradient_anim_left = (val == "1"); return; }
		if (key == "g_esp_glow_gradient_anim_right") { ui_state::entities::g_esp_glow_gradient_anim_right = (val == "1"); return; }
		if (key == "g_esp_glow_gradient_animated") { ui_state::entities::g_esp_glow_gradient_anim_left = (val == "1"); ui_state::entities::g_esp_glow_gradient_anim_right = false; return; }
		if (key == "g_esp_weapon") { ui_state::entities::g_esp_weapon = (val == "1"); return; }
		if (key == "g_esp_armor") { ui_state::entities::g_esp_armor = (val == "1"); return; }
		if (key == "g_esp_avatar") { ui_state::entities::g_esp_avatar = (val == "1"); return; }
		if (key == "g_esp_grenades") { ui_state::entities::g_esp_grenades = (val == "1"); return; }
		if (key == "g_esp_grenade_show_timer") { ui_state::entities::g_esp_grenade_show_timer = (val == "1"); return; }
		if (key == "g_esp_grenade_show_radius") { ui_state::entities::g_esp_grenade_show_radius = (val == "1"); return; }
		if (key == "g_esp_grenade_particles") { ui_state::entities::g_esp_grenade_particles = (val == "1"); return; }
		if (key == "g_esp_grenade_trails") { ui_state::entities::g_esp_grenade_trails = (val == "1"); return; }
		if (key == "g_esp_grenade_trail_particles") { ui_state::entities::g_esp_grenade_trail_particles = (val == "1"); return; }
		if (key == "g_esp_grenade_trail_length_sec") { ui_state::entities::g_esp_grenade_trail_length_sec = (float)atof(val.c_str()); return; }
		if (key == "g_esp_grenade_trail_color") { parse_f4(val, ui_state::entities::g_esp_grenade_trail_color); return; }
		if (key == "g_esp_grenade_trail_color_grad") { parse_f4(val, ui_state::entities::g_esp_grenade_trail_color_grad); return; }
		if (key == "g_esp_hp_gradient") { ui_state::entities::g_esp_hp_gradient = (val == "1"); return; }
		if (key == "g_esp_hp_show_text") { ui_state::entities::g_esp_hp_show_text = (val == "1"); return; }
		if (key == "g_esp_hp_glow") { ui_state::entities::g_esp_hp_glow = (val == "1"); return; }
		if (key == "g_esp_hp_glow_gradient") { ui_state::entities::g_esp_hp_glow_gradient = (val == "1"); return; }
		if (key == "g_esp_armor_gradient") { ui_state::entities::g_esp_armor_gradient = (val == "1"); return; }
		if (key == "g_esp_armor_show_text") { ui_state::entities::g_esp_armor_show_text = (val == "1"); return; }
		if (key == "g_esp_armor_glow") { ui_state::entities::g_esp_armor_glow = (val == "1"); return; }
		if (key == "g_esp_armor_glow_gradient") { ui_state::entities::g_esp_armor_glow_gradient = (val == "1"); return; }
		if (key == "g_esp_corner_box") { ui_state::entities::g_esp_corner_box = (val == "1"); return; }
		if (key == "g_esp_filled") { ui_state::entities::g_esp_filled = (val == "1"); return; }
		if (key == "g_esp_snaplines_top") { ui_state::entities::g_esp_snaplines_top = (val == "1"); return; }
		if (key == "g_esp_snaplines_bottom") { ui_state::entities::g_esp_snaplines_bottom = (val == "1"); return; }
		if (key == "g_esp_circle") { ui_state::entities::g_esp_circle = (val == "1"); return; }
		if (key == "g_esp_oof_arrows") { ui_state::entities::g_esp_oof_arrows = (val == "1"); return; }
		if (key == "g_esp_money") { ui_state::entities::g_esp_money = (val == "1"); return; }
		if (key == "g_esp_ping") { ui_state::entities::g_esp_ping = (val == "1"); return; }
		if (key == "g_esp_name_gradient") { ui_state::entities::g_esp_name_gradient = (val == "1"); return; }
		if (key == "g_esp_name_gradient_anim_left") { ui_state::entities::g_esp_name_gradient_anim_left = (val == "1"); return; }
		if (key == "g_esp_name_gradient_anim_right") { ui_state::entities::g_esp_name_gradient_anim_right = (val == "1"); return; }
		if (key == "g_esp_name_gradient_animated") { ui_state::entities::g_esp_name_gradient_anim_left = (val == "1"); ui_state::entities::g_esp_name_gradient_anim_right = false; return; }
		if (key == "g_esp_weapon_gradient_anim_left") { ui_state::entities::g_esp_weapon_gradient_anim_left = (val == "1"); return; }
		if (key == "g_esp_weapon_gradient_anim_right") { ui_state::entities::g_esp_weapon_gradient_anim_right = (val == "1"); return; }
		if (key == "g_esp_weapon_gradient_animated") { ui_state::entities::g_esp_weapon_gradient_anim_left = (val == "1"); ui_state::entities::g_esp_weapon_gradient_anim_right = false; return; }
		if (key == "g_esp_weapon_glow") { ui_state::entities::g_esp_weapon_glow = (val == "1"); return; }
		if (key == "g_esp_weapon_glow_gradient") { ui_state::entities::g_esp_weapon_glow_gradient = (val == "1"); return; }
		if (key == "g_esp_name_glow") { ui_state::entities::g_esp_name_glow = (val == "1"); return; }
		if (key == "g_esp_name_glow_gradient") { ui_state::entities::g_esp_name_glow_gradient = (val == "1"); return; }
		if (key == "g_esp_money_glow") { ui_state::entities::g_esp_money_glow = (val == "1"); return; }
		if (key == "g_esp_money_glow_gradient") { ui_state::entities::g_esp_money_glow_gradient = (val == "1"); return; }
		if (key == "g_esp_ping_glow") { ui_state::entities::g_esp_ping_glow = (val == "1"); return; }
		if (key == "g_esp_ping_glow_gradient") { ui_state::entities::g_esp_ping_glow_gradient = (val == "1"); return; }
		if (key == "g_esp_money_gradient") { ui_state::entities::g_esp_money_gradient = (val == "1"); return; }
		if (key == "g_esp_money_gradient_anim_left") { ui_state::entities::g_esp_money_gradient_anim_left = (val == "1"); return; }
		if (key == "g_esp_money_gradient_anim_right") { ui_state::entities::g_esp_money_gradient_anim_right = (val == "1"); return; }
		if (key == "g_esp_money_gradient_animated") { ui_state::entities::g_esp_money_gradient_anim_left = (val == "1"); ui_state::entities::g_esp_money_gradient_anim_right = false; return; }
		if (key == "g_esp_ping_gradient") { ui_state::entities::g_esp_ping_gradient = (val == "1"); return; }
		if (key == "g_esp_ping_gradient_anim_left") { ui_state::entities::g_esp_ping_gradient_anim_left = (val == "1"); return; }
		if (key == "g_esp_ping_gradient_anim_right") { ui_state::entities::g_esp_ping_gradient_anim_right = (val == "1"); return; }
		if (key == "g_esp_ping_gradient_animated") { ui_state::entities::g_esp_ping_gradient_anim_left = (val == "1"); ui_state::entities::g_esp_ping_gradient_anim_right = false; return; }
		if (key == "g_esp_armor_flag_gradient") { ui_state::entities::g_esp_armor_flag_gradient = (val == "1"); return; }
		if (key == "g_esp_armor_flag_gradient_anim_left") { ui_state::entities::g_esp_armor_flag_gradient_anim_left = (val == "1"); return; }
		if (key == "g_esp_armor_flag_gradient_anim_right") { ui_state::entities::g_esp_armor_flag_gradient_anim_right = (val == "1"); return; }
		if (key == "g_esp_armor_flag_gradient_animated") { ui_state::entities::g_esp_armor_flag_gradient_anim_left = (val == "1"); ui_state::entities::g_esp_armor_flag_gradient_anim_right = false; return; }
		if (key == "g_esp_armor_flag_glow") { ui_state::entities::g_esp_armor_flag_glow = (val == "1"); return; }
		if (key == "g_esp_armor_flag_glow_gradient") { ui_state::entities::g_esp_armor_flag_glow_gradient = (val == "1"); return; }
		if (key == "g_esp_armor_flag_force") { ui_state::entities::g_esp_armor_flag_force = (val == "1"); return; }
		if (key == "g_esp_device_enabled") { ui_state::entities::g_esp_device_enabled = (val == "1"); return; }
		if (key == "g_esp_device_gradient_anim_left") { ui_state::entities::g_esp_device_gradient_anim_left = (val == "1"); return; }
		if (key == "g_esp_device_gradient_anim_right") { ui_state::entities::g_esp_device_gradient_anim_right = (val == "1"); return; }
		if (key == "g_esp_switching") { ui_state::entities::g_esp_switching = (val == "1"); return; }
		if (key == "g_esp_switching_gradient") { ui_state::entities::g_esp_switching_gradient = (val == "1"); return; }
		if (key == "g_esp_switching_gradient_anim_left") { ui_state::entities::g_esp_switching_gradient_anim_left = (val == "1"); return; }
		if (key == "g_esp_switching_gradient_anim_right") { ui_state::entities::g_esp_switching_gradient_anim_right = (val == "1"); return; }
		if (key == "g_esp_switching_gradient_animated") { ui_state::entities::g_esp_switching_gradient_anim_left = (val == "1"); ui_state::entities::g_esp_switching_gradient_anim_right = false; return; }
		if (key == "g_kill_animated") { ui_state::entities::g_kill_animated = (val == "1"); return; }
		if (key == "g_kill_fade_duration_sec") { ui_state::entities::g_kill_fade_duration_sec = (float)atof(val.c_str()); return; }
		if (key == "g_soul_animation") { ui_state::entities::g_soul_animation = (val == "1"); return; }
		if (key == "g_soul_gradient") { ui_state::entities::g_soul_gradient = (val == "1"); return; }
		if (key == "g_soul_gradient_anim_left") { ui_state::entities::g_soul_gradient_anim_left = (val == "1"); return; }
		if (key == "g_soul_gradient_anim_right") { ui_state::entities::g_soul_gradient_anim_right = (val == "1"); return; }
		if (key == "g_soul_gradient_animated") { ui_state::entities::g_soul_gradient_anim_left = (val == "1"); ui_state::entities::g_soul_gradient_anim_right = false; return; }
		if (key == "g_soul_appear_delay_sec") { ui_state::entities::g_soul_appear_delay_sec = (float)atof(val.c_str()); return; }
		if (key == "g_soul_lifetime_sec") { ui_state::entities::g_soul_lifetime_sec = (float)atof(val.c_str()); return; }
		if (key == "g_esp_smooth_elements") { ui_state::entities::g_esp_smooth_elements = (val == "1"); return; }
		if (key == "g_esp_smoke_radius_m") { ui_state::entities::g_esp_smoke_radius_m = (float)atof(val.c_str()); return; }
		if (key == "g_esp_molotov_radius_m") { ui_state::entities::g_esp_molotov_radius_m = (float)atof(val.c_str()); return; }
		if (key == "g_esp_glow_radius") { ui_state::entities::g_esp_glow_radius = (float)atof(val.c_str()); return; }
		if (key == "g_aimbot_enabled") { ui_state::entities::g_aimbot_enabled = (val == "1"); return; }
		if (key == "g_aimbot_fov") { ui_state::entities::g_aimbot_fov = (float)atof(val.c_str()); return; }
		if (key == "g_aimbot_fov_display") { ui_state::entities::g_aimbot_fov_display = (val == "1"); return; }
		if (key == "g_aimbot_key") { ui_state::entities::g_aimbot_key = atoi(val.c_str()); return; }
		if (key == "g_aimbot_mode") { ui_state::entities::g_aimbot_mode = atoi(val.c_str()); return; }
		if (key == "g_aimbot_target") { ui_state::entities::g_aimbot_target = atoi(val.c_str()); return; }
		if (key == "g_silent_enabled") { ui_state::extra::g_silent_enabled = (val == "1"); return; }
		if (key == "g_silent_autofire") { ui_state::extra::g_silent_autofire = (val == "1"); return; }
		if (key == "g_silent_fov") { ui_state::extra::g_silent_fov = (float)atof(val.c_str()); return; }
		if (key == "g_silent_target_bone") { ui_state::extra::g_silent_target_bone = atoi(val.c_str()); return; }
		if (key == "g_silent_fov_display") { ui_state::extra::g_silent_fov_display = (val == "1"); return; }
		if (key == "g_silent_key") { ui_state::extra::g_silent_key = atoi(val.c_str()); return; }
		if (key == "g_silent_mode") { ui_state::extra::g_silent_mode = atoi(val.c_str()); return; }
		if (key == "g_aimbot_smooth") { ui_state::entities::g_aimbot_smooth = (float)atof(val.c_str()); return; }
		if (key == "g_trigger_enabled") { ui_state::trigger::g_trigger_enabled = (val == "1"); return; }
		if (key == "g_trigger_key_vk") { ui_state::trigger::g_trigger_key_vk = atoi(val.c_str()); return; }
		if (key == "g_trigger_mode") { ui_state::trigger::g_trigger_mode = atoi(val.c_str()); return; }
		if (key == "g_misc_stop_enabled") { ui_state::misc::g_misc_stop_enabled = (val == "1"); return; }
		if (key == "g_misc_stop_key") { ui_state::misc::g_misc_stop_key = atoi(val.c_str()); return; }
		if (key == "g_misc_teleport_enabled") { ui_state::misc::g_misc_teleport_enabled = (val == "1"); return; }
		if (key == "g_misc_teleport_key") { ui_state::misc::g_misc_teleport_key = atoi(val.c_str()); return; }
		if (key == "g_misc_show_material") { ui_state::misc::g_misc_show_material = (val == "1"); return; }
		if (key == "g_misc_watermark") { ui_state::misc::g_misc_watermark = (val == "1"); return; }
		if (key == "g_misc_keybinds") { ui_state::misc::g_misc_keybinds = (val == "1"); return; }
		if (key == "Drawing_b_obs_bypass") { ui_state::world::b_obs_bypass = (val == "1"); return; }
		if (key == "Drawing_night_mode") { ui_state::world::night_mode = (val == "1"); return; }
		if (key == "Drawing_world_fog_enable") { ui_state::world::world_fog_enable = (val == "1"); return; }
		if (key == "Drawing_world_fog_debug") { ui_state::world::world_fog_debug = (val == "1"); return; }
		if (key == "Drawing_world_fog_start") { ui_state::world::world_fog_start = (float)atof(val.c_str()); return; }
		if (key == "Drawing_world_fog_end") { ui_state::world::world_fog_end = (float)atof(val.c_str()); return; }
		if (key == "Drawing_world_fog_density") { ui_state::world::world_fog_density = (float)atof(val.c_str()); return; }
		if (key == "map_g_enabled") { ui_state::maps::g_enabled = (val == "1"); return; }
		if (key == "map_g_collider_map_index") { ui_state::maps::g_collider_map_index = atoi(val.c_str()); return; }
		if (key == "accent") {
			float t[4]; parse_f4(val, t);
			alice::CThemes::current.colors.accent = ImVec4(t[0], t[1], t[2], t[3]);
			return;
		}
		if (key == "Drawing_world_fog_color") { parse_f4(val, ui_state::world::world_fog_color); return; }
		if (key == "g_esp_color_box") { parse_f4(val, ui_state::entities::g_esp_color_box); return; }
		if (key == "g_esp_color_box_grad") { parse_f4(val, ui_state::entities::g_esp_color_box_grad); return; }
		if (key == "g_esp_color_name") { parse_f4(val, ui_state::entities::g_esp_color_name); return; }
		if (key == "g_esp_color_hp_full") { parse_f4(val, ui_state::entities::g_esp_color_hp_full); return; }
		if (key == "g_esp_color_hp_low") { parse_f4(val, ui_state::entities::g_esp_color_hp_low); return; }
		if (key == "g_esp_color_armor") { parse_f4(val, ui_state::entities::g_esp_color_armor); return; }
		if (key == "g_esp_color_armor_low") { parse_f4(val, ui_state::entities::g_esp_color_armor_low); return; }
		if (key == "g_esp_color_weapon") { parse_f4(val, ui_state::entities::g_esp_color_weapon); return; }
		if (key == "g_esp_color_weapon_grad") { parse_f4(val, ui_state::entities::g_esp_color_weapon_grad); return; }
		if (key == "g_esp_color_skeleton") { parse_f4(val, ui_state::entities::g_esp_color_skeleton); return; }
		if (key == "g_esp_color_hitbox_fill") { parse_f4(val, ui_state::entities::g_esp_color_hitbox_fill); return; }
		if (key == "g_esp_color_hitbox_fill_bottom") { parse_f4(val, ui_state::entities::g_esp_color_hitbox_fill_bottom); return; }
		if (key == "g_esp_glow_color") { parse_f4(val, ui_state::entities::g_esp_glow_color); return; }
		if (key == "g_esp_glow_color_bottom") { parse_f4(val, ui_state::entities::g_esp_glow_color_bottom); return; }
		if (key == "g_esp_smoke_circle_outline_color") { parse_f4(val, ui_state::entities::g_esp_smoke_circle_outline_color); return; }
		if (key == "g_esp_molotov_circle_outline_color") { parse_f4(val, ui_state::entities::g_esp_molotov_circle_outline_color); return; }
		if (key == "g_esp_color_snaplines") { parse_f4(val, ui_state::entities::g_esp_color_snaplines); return; }
		if (key == "g_esp_color_circle") { parse_f4(val, ui_state::entities::g_esp_color_circle); return; }
		if (key == "g_esp_color_arrows") { parse_f4(val, ui_state::entities::g_esp_color_arrows); return; }
		if (key == "g_esp_color_money") { parse_f4(val, ui_state::entities::g_esp_color_money); return; }
		if (key == "g_esp_color_ping") { parse_f4(val, ui_state::entities::g_esp_color_ping); return; }
		if (key == "g_esp_color_money_grad") { parse_f4(val, ui_state::entities::g_esp_color_money_grad); return; }
		if (key == "g_esp_color_ping_grad") { parse_f4(val, ui_state::entities::g_esp_color_ping_grad); return; }
		if (key == "g_esp_color_device") { parse_f4(val, ui_state::entities::g_esp_color_device); return; }
		if (key == "g_esp_color_device_grad") { parse_f4(val, ui_state::entities::g_esp_color_device_grad); return; }
		if (key == "g_esp_color_switching") { parse_f4(val, ui_state::entities::g_esp_color_switching); return; }
		if (key == "g_esp_color_switching_grad") { parse_f4(val, ui_state::entities::g_esp_color_switching_grad); return; }
		if (key == "g_esp_color_name_grad") { parse_f4(val, ui_state::entities::g_esp_color_name_grad); return; }
		if (key == "g_esp_color_armor_flag") { parse_f4(val, ui_state::entities::g_esp_color_armor_flag); return; }
		if (key == "g_esp_color_armor_flag_grad") { parse_f4(val, ui_state::entities::g_esp_color_armor_flag_grad); return; }
		if (key == "g_esp_color_bullet_tracer") { parse_f4(val, ui_state::entities::g_esp_color_bullet_tracer); return; }
		if (key == "g_esp_grenade_icon_color") { parse_f4(val, ui_state::entities::g_esp_grenade_icon_color); return; }
		if (key == "g_esp_grenade_icon_color_grad") { parse_f4(val, ui_state::entities::g_esp_grenade_icon_color_grad); return; }
		if (key == "g_silent_fov_color") { parse_f4(val, ui_state::extra::g_silent_fov_color); return; }
		if (key == "ui_widget_watermark") {
			if (everness::ctx && everness::ctx->alice && everness::ctx->alice->widgets && everness::ctx->alice->widgets->watermark)
				everness::ctx->alice->widgets->watermark->enabled = (val == "1");
			return;
		}
		if (key == "ui_widget_keybinds") {
			if (everness::ctx && everness::ctx->alice && everness::ctx->alice->widgets && everness::ctx->alice->widgets->keybinds)
				everness::ctx->alice->widgets->keybinds->enabled = (val == "1");
			return;
		}
		if (key == "ui_widget_hitlogs") {
			if (everness::ctx && everness::ctx->alice && everness::ctx->alice->widgets && everness::ctx->alice->widgets->hitLogs)
				everness::ctx->alice->widgets->hitLogs->enabled = (val == "1");
			return;
		}
		if (key == "bb") {
			apply_bind_line(val);
			return;
		}
	}

	static bool write_encrypted(const std::wstring& path, const std::string& plain)
	{
		uint8_t salt[16];
		HCRYPTPROV prov = 0;
		if (!CryptAcquireContextW(&prov, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
			for (int i = 0; i < 16; ++i) salt[i] = (uint8_t)(rand() ^ (i * 17));
		} else {
			CryptGenRandom(prov, 16, salt);
			CryptReleaseContext(prov, 0);
		}
		std::vector<uint8_t> buf(plain.begin(), plain.end());
		xor_crypt(buf.data(), buf.size(), salt);
		std::ofstream f(std::filesystem::path(path), std::ios::binary);
		if (!f) { set_err("open write"); return false; }
		f.write((const char*)k_magic, 8);
		f.write((const char*)salt, 16);
		uint32_t sz = (uint32_t)buf.size();
		f.write((const char*)&sz, 4);
		f.write((const char*)buf.data(), buf.size());
		return true;
	}

	static bool read_decrypt(const std::wstring& path, std::string& out_plain)
	{
		std::ifstream f(std::filesystem::path(path), std::ios::binary | std::ios::ate);
		if (!f) { set_err("open read"); return false; }
		size_t fsz = (size_t)f.tellg();
		f.seekg(0);
		if (fsz < 8 + 16 + 4) { set_err("short file"); return false; }
		char mg[8];
		f.read(mg, 8);
		if (memcmp(mg, k_magic, 8) != 0) { set_err("bad magic"); return false; }
		uint8_t salt[16];
		f.read((char*)salt, 16);
		uint32_t sz = 0;
		f.read((char*)&sz, 4);
		if (fsz < 8 + 16 + 4 + sz || sz > 10 * 1024 * 1024) { set_err("size"); return false; }
		std::vector<uint8_t> buf(sz);
		f.read((char*)buf.data(), sz);
		xor_crypt(buf.data(), buf.size(), salt);
		out_plain.assign((char*)buf.data(), buf.size());
		return true;
	}

	static std::wstring path_cfg(const std::string& name_no_ext)
	{
		std::wstring dir = get_config_dir_w();
		std::string safe = sanitize_config_name(name_no_ext.c_str());
		if (safe.empty()) return L"";
		std::wstring n(safe.begin(), safe.end());
		return dir + L"\\" + n + L".cfg";
	}

	std::vector<std::string> list_configs()
	{
		std::vector<std::string> r;
		if (!ensure_directory()) return r;
		std::wstring dir = get_config_dir_w();
		std::wstring pat = dir + L"\\*.cfg";
		WIN32_FIND_DATAW fd;
		HANDLE h = FindFirstFileW(pat.c_str(), &fd);
		if (h == INVALID_HANDLE_VALUE) return r;
		do {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
			std::wstring w = fd.cFileName;
			if (w.size() <= 4) continue;
			if (_wcsicmp(w.c_str() + w.size() - 4, L".cfg") != 0) continue;
			w.resize(w.size() - 4);
			char u[512];
			WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, u, sizeof(u), nullptr, nullptr);
			r.push_back(u);
		} while (FindNextFileW(h, &fd));
		FindClose(h);
		std::sort(r.begin(), r.end());
		return r;
	}

	bool create_config(const char* name_no_ext)
	{
		std::string s = sanitize_config_name(name_no_ext);
		if (s.empty()) { set_err("name"); return false; }
		if (!ensure_directory()) return false;
		std::wstring p = path_cfg(s);
		if (p.empty()) return false;
		if (GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES) { set_err("exists"); return false; }
		return save_config(s.c_str());
	}

	bool save_config(const char* name_no_ext)
	{
		std::string s = sanitize_config_name(name_no_ext);
		if (s.empty()) { set_err("name"); return false; }
		if (!ensure_directory()) return false;
		std::wstring p = path_cfg(s);
		std::string plain = build_plaintext();
		if (!write_encrypted(p, plain)) return false;
		set_err("ok");
		return true;
	}

	bool load_config(const char* name_no_ext)
	{
		std::string s = sanitize_config_name(name_no_ext);
		if (s.empty()) { set_err("name"); return false; }
		std::wstring p = path_cfg(s);
		if (GetFileAttributesW(p.c_str()) == INVALID_FILE_ATTRIBUTES) { set_err("not found"); return false; }
		std::string plain;
		if (!read_decrypt(p, plain)) return false;
		clear_config_binds();
		std::istringstream iss(plain);
		std::string line;
		while (std::getline(iss, line)) {
			if (line.empty() || line.back() == '\r') {
				if (!line.empty() && line.back() == '\r') line.pop_back();
			}
			size_t eq = line.find('=');
			if (eq == std::string::npos) continue;
			std::string k = line.substr(0, eq);
			std::string v = line.substr(eq + 1);
			apply_line(k, v);
		}
		if (ui_state::maps::g_collider_map_count > 0) {
			if (ui_state::maps::g_collider_map_index < 0 || ui_state::maps::g_collider_map_index >= ui_state::maps::g_collider_map_count)
				ui_state::maps::g_collider_map_index = 0;
			// Local selection only; no map loader exists.
		}
		set_err("ok");
		return true;
	}

	bool delete_config(const char* name_no_ext)
	{
		std::string s = sanitize_config_name(name_no_ext);
		if (s.empty()) { set_err("name"); return false; }
		std::wstring p = path_cfg(s);
		if (GetFileAttributesW(p.c_str()) == INVALID_FILE_ATTRIBUTES) { set_err("not found"); return false; }
		if (!DeleteFileW(p.c_str())) { set_err("delete"); return false; }
		set_err("ok");
		return true;
	}

	void open_config_directory()
	{
		if (!ensure_directory()) return;
		std::wstring dir = get_config_dir_w();
		ShellExecuteW(nullptr, L"open", dir.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}
}
