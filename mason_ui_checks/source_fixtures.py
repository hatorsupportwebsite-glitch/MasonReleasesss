"""Minimal structural fixtures from inspected source regions, NOT the full repo."""
import json

UI = r'''#include "ui.hpp"
void CUserInterface::MenuSettings::render(const std::function<void()>& back_task) {
    static std::vector<std::string> themeNames;
    static std::vector<Theme*> themePtrs;
    static int currentThemeIndex = 0;
    for (int i=0; i < themePtrs.size(); ++i) {
        if (themePtrs[i] == &CThemes::current) { currentThemeIndex=i; break; }
    }
    if (CUIElements::combo(ElemType::Begin, "Theme", &currentThemeIndex, themeNames)) {
        CThemes::current = *themePtrs[currentThemeIndex];
    }
    CUIElements::toggle(ElemType::Middle, "Main glow", &everness::c::get<bool>(everness::ctx->config.uiEnableGlowMain));
    CUIElements::toggle(ElemType::Middle, "Logo glow", &everness::c::get<bool>(everness::ctx->config.uiEnableGlowLogo));
}
void CUserInterface::init() {
    auto& style=GetStyle();
    style.Scale=1;
    m_tabs = {
      {.name="VISUAL", .type=TabType::Category},
      {.name="Entities", .callback=tabs::callVisual},
      {.name="World", .callback=tabs::callWorld, .subTabs = std::vector<SubTab>{{"Visual", tabs::callWorld}, {"Misc", tabs::callWorldMisc}}},
      {.name="COMBAT", .type=TabType::Category},
      {.name="Legitbot", .callback=tabs::callLegitbot, .subTabs = std::vector<SubTab>{{"Aimbot", tabs::callLegitbot}, {"Triggerbot", tabs::callLegitbotTrigger}}},
      {.name="CLOUD", .type=TabType::Category},
      {.name="Skins", .callback=tabs::callSkins},
      {.name="Configs", .callback=tabs::callConfig}
    };
    m_selectedTab = 1;
}
void CUserInterface::render() {
    auto& io=GetIO();
    auto& style=GetStyle();
    SetNextWindowSize(m_windowSize);
    SetNextWindowPos(io.DisplaySize / 2 - m_windowSize / 2, ImGuiCond_Once);
    Begin("@alicegang", nullptr, ImGuiWindowFlags_NoDecoration);
    BeginChild("##top_area", {GetContentRegionAvail().x, 48}, 0, ImGuiWindowFlags_AlwaysUseWindowPadding);
    {
        if (m_selectedTab == 1) {
            saveButton({70, GetContentRegionAvail().y}, [] {
                everness::ctx->alice->alerts->create("Error", {260, 200}, [](Alert& alert) {
                    CAlerts::renderContents(GetCursorScreenPos(), "Something went wrong...", nullptr, GetWindowSize().x);
                    if (CAlerts::button("OK", {240, 44}, true)) alert.close();
                });
            });
            SameLine(0, 10);
        }
        PushStyleVar(ImGuiStyleVar_Alpha, m_subTabsAreaOpacity.get<float>() * menu_opacity);
        for (auto& subtab : m_tabs[m_selectedTab].subTabs.value_or(std::vector<SubTab>{})) {
            renderSubTab(&subtab, false, 0);
            SameLine(0,0);
        }
        PopStyleVar();
        SetCursorPos({GetWindowSize().x - 28 - 10, 10});
        if (InvisibleButton("##SETTINGS", GetContentRegionAvail())) m_settings.presented=true;
        EndChild();
    }
    if (m_selectedTab == kConfigsTab) m_callback(0);
    else { m_callback(1); m_callback(2); }
    End();
}
'''

VISUAL = r'''#include "core.hpp"
#include "ui_state.hpp"
using namespace ImGui;
using namespace alice;
using namespace everness;
static void esp_popup_gradient_anim_lr(bool& left, bool& right) {
    PopupEntries::toggle(std::nullopt, "Left Animated", &left);
    if (left) right=false;
}
void alice::tabs::callVisual(int child) {
    if (child==1) {
        static CPopup pop_chams(180, "Chams", std::nullopt);
        CUIElements::toggle(ElemType::Middle, "Chams##ent_chams", &ui_state::entities::g_esp_hitbox_outline_fill,
          {(ImVec4*)ui_state::entities::g_esp_color_hitbox_fill, (ImVec4*)ui_state::entities::g_esp_color_hitbox_fill_bottom},
          std::make_pair(&pop_chams, [&]() {
            if(pop_chams.begin()) {
              esp_popup_gradient_anim_lr(ui_state::entities::g_esp_chams_gradient_anim_left, ui_state::entities::g_esp_chams_gradient_anim_right);
              pop_chams.end();
            }
          }));
    } else if (child==2) {
        static CPopup pop_hitsparks(180, "Hit Sparks", std::nullopt);
        CUIElements::toggle(ElemType::Middle, "Corner Box##ent2_corner", &ui_state::entities::g_esp_corner_box);
        CUIElements::toggle(ElemType::Middle, "Filled Box##ent2_fill", &ui_state::entities::g_esp_filled, {(ImVec4*)ui_state::entities::g_esp_color_hitbox_fill});
        CUIElements::toggle(ElemType::Middle, "Step Circle##ent2_circle", &ui_state::entities::g_esp_circle, {(ImVec4*)ui_state::entities::g_esp_color_circle});
        CUIElements::toggle(ElemType::Middle, "Hit Sparks##ent2_hitsparks", &ui_state::entities::g_esp_hit_sparks, {}, std::make_pair(&pop_hitsparks, [&]() {
            if(pop_hitsparks.begin()) { PopupEntries::sliderInt(std::nullopt, "Count", &ui_state::entities::g_esp_hit_sparks_count, 4, 80); pop_hitsparks.end(); }
        }));
    }
}
void alice::tabs::callWorld(int child) {
    if (child==1) {
        CUIElements::toggle(ElemType::Middle, "Grenade Trails##w_gren_trails", &ui_state::entities::g_esp_grenade_trails);
        CUIElements::color(ElemType::Middle, "Smoke Color##w_smoke", {(ImVec4*)ui_state::entities::g_esp_smoke_circle_outline_color});
        CUIElements::color(ElemType::End, "Molotov Color##w_molo", {(ImVec4*)ui_state::entities::g_esp_molotov_circle_outline_color});
    } else if (child==2) {
        CUIElements::toggle(ElemType::Begin, "Night Mode##w_night", &ui_state::world::night_mode);
    }
}
void alice::tabs::callWorldMisc(int child) {
    if (child==1) { CUIElements::toggle(ElemType::Begin, "Teleport##misc_tp", &ui_state::misc::g_misc_teleport_enabled); }
}
void alice::tabs::callLegitbot(int child) {
    static CPopup pop_aim_binder(160, "Aimbot binds", std::nullopt);
    if (child==1) {
        CUIElements::toggle(ElemType::Begin, "Aimbot##aim_en", &ui_state::entities::g_aimbot_enabled, {}, std::make_pair(&pop_aim_binder, []() {
            if(pop_aim_binder.begin()) { PopupEntries::button(std::nullopt, "Add hotkey"); pop_aim_binder.end(); }
        }));
        CUIElements::toggle(ElemType::Middle, "Show Fov##aim_fov_show", &ui_state::entities::g_aimbot_fov_display);
        static const std::vector<std::string> bones={"Head", "Neck", "Body"};
        CUIElements::combo(ElemType::End, "Bones##aim_bones", &ui_state::entities::g_aimbot_target, bones);
    } else if (child==2) {
    }
}
void alice::tabs::callLegitbotTrigger(int child) {
    CUIElements::toggle(ElemType::Begin, "Triggerbot##trig_en", &ui_state::trigger::g_trigger_enabled);
}
void alice::tabs::callSkins(int child) {}
'''

CONFIGS = r'''#include "configs.h"
#include "ui_state.hpp"
namespace configs {
static std::string build_plaintext() {
    std::ostringstream o;
    o << "v=2\n";
    bf(o, "g_silent_enabled", ui_state::extra::g_silent_enabled);
    bf(o, "g_silent_autofire", ui_state::extra::g_silent_autofire);
    f4(o, "accent", &alice::CThemes::current.colors.accent.x);
    append_bind_lines(o);
    return o.str();
}
static void apply_line(const std::string& key, const std::string& val) {
    if(key=="g_aimbot_target") { ui_state::entities::g_aimbot_target=atoi(val.c_str()); return; }
}
static bool write_encrypted(const std::wstring& path, const std::string& plain) {
    std::ofstream f(std::filesystem::path(path), std::ios::binary);
    if (!f) { set_err("open write"); return false; }
    f.write(plain.data(), plain.size());
    return true;
}
bool save_config(const char* name_no_ext) {
    const auto plain=build_plaintext();
    if (!write_encrypted(path_cfg(name_no_ext), plain)) return false;
    return true;
}
bool load_config(const char* name_no_ext) {
    std::wstring p=path_cfg(name_no_ext);
    std::string plain;
    if (!read_decrypt(p, plain)) return false;
    clear_config_binds();
    std::istringstream iss(plain);
    std::string line;
    while(std::getline(iss,line)) {
        size_t eq=line.find('=');
        if(eq==std::string::npos) continue;
        std::string k=line.substr(0,eq), v=line.substr(eq+1);
        apply_line(k,v);
    }
    return true;
}
}
'''

MENU = r'''#include "menu.h"
namespace mason {
bool InitializeMenu(ID3D11Device* device, ID3D11DeviceContext* context) {
    everness::ctx->alice->ui->init();
    everness::ctx->alice->themes->addDefaults();
    SetMenuOpen(true);
    return alice::CUserInterface::logo && ImGui::GetIO().FontDefault;
}
void SetMenuOpen(bool v) { if(everness::ctx) everness::ctx->alice->ui->isPresented()=v; }
}
'''

OVERLAY = r'''#include "overlay.h"
int mason::RunOverlay(bool smoke_test) {
  while(!done) {
    ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();
    RenderMenu();ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  }
  return 0;
}
'''


def make_project(root, updater, crlf=False):
    root.mkdir(parents=True, exist_ok=True)
    sources = {updater.UI:UI, updater.VISUAL:VISUAL, updater.CONFIGS:CONFIGS, updater.MENU:MENU, updater.OVERLAY:OVERLAY}
    for name, content in sources.items():
        path=root/name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content.replace('\n','\r\n').encode() if crlf else content.encode())
    (root/'CMakeLists.txt').write_text('# Test fixture, not a real build project.\n')
    manifest=root/'tests/ui_preservation_manifest.json'
    manifest.parent.mkdir(exist_ok=True)
    manifest.write_text(json.dumps([
        {'path':updater.UI.removeprefix('product/'),'modified':True,'original_sha256':'unchanged-ui-baseline'},
        {'path':updater.VISUAL.removeprefix('product/'),'modified':True,'original_sha256':'unchanged-tabs-baseline'}
    ]))
    return sources
