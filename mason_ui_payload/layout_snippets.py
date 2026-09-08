"""C++ fragments for the existing Mason standalone UI; no game functionality."""

SIDEBAR = r'''m_tabs = {
        { .name = "COMMON", .type = TabType::Category },
        { .name = "Entities", .icon = ICON_FA_USER, .type = TabType::Tab, .callback = tabs::callVisual },
        { .name = "World", .icon = ICON_FA_GLOBE, .type = TabType::Tab, .callback = tabs::callWorld },
        { .name = "Inventory", .icon = ICON_FA_SHIRT, .type = TabType::Tab, .callback = tabs::callSkins },
        { .name = "COMBAT", .type = TabType::Category },
        { .name = "Aimbot", .icon = ICON_FA_DROPLET, .type = TabType::Tab, .callback = tabs::callLegitbot },
        { .name = "CLOUD", .type = TabType::Category },
        { .name = "Configs", .icon = ICON_FA_CLOUD, .type = TabType::Tab, .callback = tabs::callConfig }
    };'''

SAVE_AREA = r'''
        saveButton( { 70, GetContentRegionAvail( ).y }, [] {
            const bool saved = configs::save_config("autosave");
            const std::string message = saved ? "Saved to autosave.cfg" : configs::last_error();
            everness::ctx->alice->alerts->create( saved ? "Saved" : "Save failed",
                { 300 * GetStyle( ).Scale, 160 * GetStyle( ).Scale },
                [message]( Alert& alert ) {
                    PushStyleColor( ImGuiCol_Text, ImColor( CThemes::current.colors.textDim ).Value );
                    CAlerts::renderContents( GetCursorScreenPos( ) + ImVec2( 5 * GetStyle( ).Scale, 0 ),
                        message.c_str(), nullptr, GetWindowSize( ).x - 10 * GetStyle( ).Scale );
                    PopStyleColor( );
                    SetCursorPosY( GetWindowSize( ).y - 44 * GetStyle( ).Scale );
                    if ( CAlerts::button( "OK", { 280 * GetStyle( ).Scale, 44 * GetStyle( ).Scale }, true ) )
                        alert.close( );
                } );
        } );

        '''

DPI_SETTING = r'''
        {
            static const std::vector<std::string> dpi_labels = { "75%", "100%", "125%", "150%", "175%", "200%" };
            static constexpr int dpi_values[] = { 75, 100, 125, 150, 175, 200 };
            int selected_dpi = 0;
            int distance = 1000;
            for (int i = 0; i < 6; ++i) {
                const int delta = std::abs(mason_ui::options.dpi_percent - dpi_values[i]);
                if (delta < distance) { distance = delta; selected_dpi = i; }
            }
            if (CUIElements::combo(ElemType::Middle, "DPI SCALE", &selected_dpi, dpi_labels))
                mason_ui::options.dpi_percent = dpi_values[selected_dpi];
        }

'''

CHOICE_HELPER = r'''
// A single selection, displayed with the same popup-row style as the other controls.
static void mason_single_choice(int& selection, const std::vector<std::string>& labels)
{
    if (labels.empty()) return;
    selection = std::clamp(selection, 0, static_cast<int>(labels.size()) - 1);
    for (int i = 0; i < static_cast<int>(labels.size()); ++i) {
        ImGui::PushID(i);
        bool selected = selection == i;
        if (PopupEntries::toggle(std::nullopt, labels[i], &selected) && selected) selection = i;
        ImGui::PopID();
    }
}

'''

AIM_LEFT = r'''
        CUIElements::toggle( ElemType::Middle, "Show Fov##aim_fov_show", &ui_state::entities::g_aimbot_fov_display );
        CUIElements::toggle( ElemType::Middle, "Silent aim##aim_silent", &ui_state::extra::g_silent_enabled );
        CUIElements::toggle( ElemType::Middle, "Aim through walls##aim_walls", &mason_ui::options.aim_through_walls );
        CUIElements::toggle( ElemType::Middle, "Automatically fire##aim_auto_fire", &ui_state::extra::g_silent_autofire );

        static const std::vector<std::string> bones = { "Head", "Neck", "Body", "Arms", "Legs", "Stomach", "Torso" };
        ui_state::entities::g_aimbot_target = std::clamp(ui_state::entities::g_aimbot_target, 0, 6);
        CUIElements::combo( ElemType::Middle, "Bones##aim_bones", &ui_state::entities::g_aimbot_target, bones );
        CUIElements::combo( ElemType::Middle, "Multi points##aim_multi_points", &mason_ui::options.multi_points, bones );

        static CPopup pop_chance( 220, "Hit chance", std::nullopt );
        CUIElements::toggle( ElemType::End, "Hit chance##aim_hit_chance", &mason_ui::options.hit_chance_enabled, {},
            std::make_pair( &pop_chance, [] {
                if (pop_chance.begin()) {
                    PopupEntries::sliderInt(std::nullopt, "Hit chance (%)", &mason_ui::options.hit_chance_percent, 0, 100);
                    pop_chance.end();
                }
            } ) );
'''

AIM_RIGHT = r'''
        static CPopup pop_hitbox( 190, "Hitbox override", std::nullopt );
        static CPopup pop_texture( 190, "Texture override", std::nullopt );
        CUIElements::toggle( ElemType::Begin, "Rapid fire##aim_rapid", &mason_ui::options.rapid_fire );
        CUIElements::toggle( ElemType::Middle, "Wallshot##aim_wallshot", &mason_ui::options.wallshot );
        CUIElements::toggle( ElemType::Middle, "No recoil##aim_no_recoil", &mason_ui::options.no_recoil );
        CUIElements::toggle( ElemType::Middle, "No spread##aim_no_spread", &mason_ui::options.no_spread );
        CUIElements::toggle( ElemType::Middle, "Hitbox override##aim_hitbox", &mason_ui::options.hitbox_override, {},
            std::make_pair( &pop_hitbox, [] {
                if (pop_hitbox.begin()) {
                    static const std::vector<std::string> hitboxes = { "Head", "Stomach", "Body", "Neck", "Arms", "Legs", "Torso" };
                    mason_single_choice(mason_ui::options.hitbox, hitboxes);
                    pop_hitbox.end();
                }
            } ) );
        CUIElements::toggle( ElemType::End, "Texture override##aim_texture", &mason_ui::options.texture_override, {},
            std::make_pair( &pop_texture, [] {
                if (pop_texture.begin()) {
                    static const std::vector<std::string> textures = { "Blood", "Grenade", "Molotov", "Glass" };
                    mason_single_choice(mason_ui::options.texture, textures);
                    pop_texture.end();
                }
            } ) );
'''

REMOVALS = r'''static CPopup pop_removals( 180, "Removals", std::nullopt );
        CUIElements::toggle( ElemType::Begin, "Removals##w_removals", &mason_ui::options.removals, {},
            std::make_pair( &pop_removals, [] {
                if (pop_removals.begin()) {
                    PopupEntries::toggle(std::nullopt, "Smoke", &mason_ui::options.remove_smoke);
                    PopupEntries::toggle(std::nullopt, "Flash", &mason_ui::options.remove_flash);
                    PopupEntries::toggle(std::nullopt, "Blood", &mason_ui::options.remove_blood);
                    PopupEntries::toggle(std::nullopt, "Team", &mason_ui::options.remove_team);
                    PopupEntries::toggle(std::nullopt, "Shake", &mason_ui::options.remove_shake);
                    pop_removals.end();
                }
            } ) );'''

CONFIG_APPLY = r'''
        if (mason_ui::read_option(key, val)) {
            if (key == "ui2_theme" && everness::ctx && everness::ctx->alice && everness::ctx->alice->themes) {
                auto& themes = everness::ctx->alice->themes->getAllThemes();
                const auto found = themes.find(mason_ui::options.theme_name);
                if (found != themes.end()) alice::CThemes::current = found->second;
            }
            return;
        }
        if (key == "ui2_glow_main") {
            if (everness::ctx) everness::c::get<bool>(everness::ctx->config.uiEnableGlowMain) = (val == "1");
            return;
        }
        if (key == "ui2_glow_logo") {
            if (everness::ctx) everness::c::get<bool>(everness::ctx->config.uiEnableGlowLogo) = (val == "1");
            return;
        }
'''

CONFIG_WRITE = r'''
        mason_ui::write_options(o);
        if (everness::ctx) {
            bf(o, "ui2_glow_main", everness::c::get<bool>(everness::ctx->config.uiEnableGlowMain));
            bf(o, "ui2_glow_logo", everness::c::get<bool>(everness::ctx->config.uiEnableGlowLogo));
        }
'''
