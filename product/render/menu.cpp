#include "menu.h"
#include "assets/logo_bytes.hpp"
#include "core.hpp"
#include "graphics/graphics.hpp"
#include "alice/fonts/fonts.hpp"
#include "alice/ui/ui.hpp"
#include "alice/ui/alerts/alerts.hpp"
#include "alice/ui/widgets/widgets.hpp"
#include "alice/ui/themes/themes.hpp"
namespace mason {
bool InitializeMenu(ID3D11Device* device,ID3D11DeviceContext* context) {
    everness::ctx=std::make_unique<everness::CCore>();
    everness::CGraphics::m_device=device;everness::CGraphics::m_deviceContext=context;
    everness::ctx->alice->ui->init();everness::ctx->alice->fonts->init();
    everness::ctx->alice->themes->addDefaults();
    alice::CUserInterface::logo=everness::ctx->graphics->loadTextureFromMemory(assets::logo_png,assets::logo_png_size,true);
    alice::CUserInterface::profile_picture=0;
    ImGui::GetIO().FontDefault=everness::ctx->alice->fonts->get("primary").ptr;
    SetMenuOpen(true);
    return alice::CUserInterface::logo && ImGui::GetIO().FontDefault;
}
void RenderMenu() {
    if(!everness::ctx)return;
    everness::ctx->alice->ui->render();everness::ctx->alice->alerts->render();everness::ctx->alice->widgets->render();
}
void ShutdownMenu() {
    alice::CUserInterface::logo=0;alice::CUserInterface::profile_picture=0;
    everness::ctx.reset();everness::c::variables.clear();
    everness::CGraphics::m_device=nullptr;everness::CGraphics::m_deviceContext=nullptr;
}
bool IsMenuOpen(){return everness::ctx && everness::ctx->alice->ui->isPresented();}
void SetMenuOpen(bool v){if(everness::ctx)everness::ctx->alice->ui->isPresented()=v;}
}
