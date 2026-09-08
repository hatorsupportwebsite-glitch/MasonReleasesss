#pragma once

// Mason UI customization v1. Local menu/configuration data only.
#include <algorithm>
#include <charconv>
#include <ostream>
#include <string>
#include <string_view>
#include <system_error>

namespace mason_ui {
struct Options {
    bool aim_through_walls = false;
    int multi_points = 0;
    bool hit_chance_enabled = false;
    int hit_chance_percent = 100;
    bool rapid_fire = false;
    bool wallshot = false;
    bool no_recoil = false;
    bool no_spread = false;
    bool hitbox_override = false;
    int hitbox = 0;
    bool texture_override = false;
    int texture = 0;
    bool removals = false;
    bool remove_smoke = false;
    bool remove_flash = false;
    bool remove_blood = false;
    bool remove_team = false;
    bool remove_shake = false;
    int dpi_percent = 100;
    std::string theme_name;
};
inline Options options;

struct FlagField { const char* key; bool Options::*member; };
inline constexpr FlagField flag_fields[] = {
    {"ui2_aim_through_walls", &Options::aim_through_walls},
    {"ui2_hit_chance_enabled", &Options::hit_chance_enabled},
    {"ui2_rapid_fire", &Options::rapid_fire},
    {"ui2_wallshot", &Options::wallshot},
    {"ui2_no_recoil", &Options::no_recoil},
    {"ui2_no_spread", &Options::no_spread},
    {"ui2_hitbox_override", &Options::hitbox_override},
    {"ui2_texture_override", &Options::texture_override},
    {"ui2_removals", &Options::removals},
    {"ui2_remove_smoke", &Options::remove_smoke},
    {"ui2_remove_flash", &Options::remove_flash},
    {"ui2_remove_blood", &Options::remove_blood},
    {"ui2_remove_team", &Options::remove_team},
    {"ui2_remove_shake", &Options::remove_shake},
};
struct IntField { const char* key; int Options::*member; int low; int high; };
inline constexpr IntField int_fields[] = {
    {"ui2_multi_points", &Options::multi_points, 0, 6},
    {"ui2_hit_chance_percent", &Options::hit_chance_percent, 0, 100},
    {"ui2_hitbox", &Options::hitbox, 0, 6},
    {"ui2_texture", &Options::texture, 0, 3},
    {"ui2_dpi_percent", &Options::dpi_percent, 75, 200},
};

inline void reset_options() { options = Options{}; }
inline void write_options(std::ostream& out) {
    for (const auto& field : flag_fields)
        out << field.key << '=' << (options.*(field.member) ? 1 : 0) << '\n';
    for (const auto& field : int_fields)
        out << field.key << '=' << std::clamp(options.*(field.member), field.low, field.high) << '\n';
    if (options.theme_name.size() <= 128 && options.theme_name.find_first_of("\r\n") == std::string::npos)
        out << "ui2_theme=" << options.theme_name << '\n';
}

// Returns true for a recognized key. Malformed values leave its default intact.
inline bool read_option(std::string_view key, std::string_view value) {
    for (const auto& field : flag_fields) {
        if (key != field.key) continue;
        if (value == "0" || value == "1") options.*(field.member) = value == "1";
        return true;
    }
    for (const auto& field : int_fields) {
        if (key != field.key) continue;
        if (value.empty()) return true;
        int parsed = 0;
        const auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
        if (result.ec == std::errc{} && result.ptr == value.data() + value.size())
            options.*(field.member) = std::clamp(parsed, field.low, field.high);
        return true;
    }
    if (key == "ui2_theme") {
        if (value.size() <= 128 && value.find_first_of("\r\n") == std::string_view::npos) {
            if (value.empty()) options.theme_name.clear();
            else options.theme_name.assign(value.data(), value.size());
        }
        return true;
    }
    return false;
}
}
