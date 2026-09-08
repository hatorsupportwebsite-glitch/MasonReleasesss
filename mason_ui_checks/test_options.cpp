#include "mason_ui_options.hpp"
#include <cassert>
#include <iostream>
#include <set>
#include <sstream>

using namespace mason_ui;
int main() {
    reset_options();
    assert(options.dpi_percent == 100 && options.hit_chance_percent == 100);
    for (const auto& field : flag_fields) assert(!(options.*(field.member)));
    std::set<std::string> keys;
    for (const auto& field : flag_fields) {
        assert(keys.insert(field.key).second);
        options.*(field.member) = true;
    }
    for (const auto& field : int_fields) {
        assert(keys.insert(field.key).second);
        options.*(field.member) = field.high;
    }
    options.theme_name = "Midnight";
    std::ostringstream saved;
    write_options(saved);
    const std::string expected = saved.str();
    reset_options();
    std::istringstream input(expected);
    std::string line;
    while (std::getline(input, line)) {
        auto eq = line.find('=');
        assert(eq != std::string::npos);
        assert(read_option(line.substr(0, eq), line.substr(eq + 1)));
    }
    std::ostringstream loaded;
    write_options(loaded);
    assert(loaded.str() == expected);
    for (const auto& field : flag_fields) assert(options.*(field.member));
    for (const auto& field : int_fields) assert(options.*(field.member) == field.high);
    assert(options.theme_name == "Midnight");

    for (const auto& field : int_fields) {
        assert(read_option(field.key, "-12345"));
        assert(options.*(field.member) == field.low);
        assert(read_option(field.key, "12345"));
        assert(options.*(field.member) == field.high);
        for (const char* bad : {"", "abc", "1junk", "1.5", "999999999999999999999999999999999"}) {
            assert(read_option(field.key, bad));
            assert(options.*(field.member) == field.high);
        }
    }
    assert(read_option("ui2_hit_chance_percent", "0") && options.hit_chance_percent == 0);
    assert(read_option("ui2_hit_chance_percent", "100") && options.hit_chance_percent == 100);
    assert(!read_option("unknown_key", "1"));
    options.rapid_fire = false;
    assert(read_option("ui2_rapid_fire", "true"));
    assert(!options.rapid_fire);
    assert(read_option("ui2_rapid_fire", "1") && options.rapid_fire);
    assert(read_option("ui2_rapid_fire", "0") && !options.rapid_fire);
    assert(read_option("ui2_theme", "Dark\nui2_rapid_fire=1"));
    assert(options.theme_name == "Midnight");
    assert(read_option("ui2_theme", std::string_view{}));
    assert(options.theme_name.empty());
    options.theme_name = "invalid\nname";
    std::ostringstream invalid;
    write_options(invalid);
    assert(invalid.str().find("ui2_theme=") == std::string::npos);

    options.dpi_percent = 999;
    options.hit_chance_percent = -42;
    std::ostringstream clamped;
    write_options(clamped);
    assert(clamped.str().find("ui2_dpi_percent=200\n") != std::string::npos);
    assert(clamped.str().find("ui2_hit_chance_percent=0\n") != std::string::npos);
    reset_options();
    assert(!options.rapid_fire && !options.removals && options.dpi_percent == 100);
    std::cout << "PASS: all option fields round-trip, ranges clamp, malformed values are rejected, and legacy defaults reset.\n";
}
