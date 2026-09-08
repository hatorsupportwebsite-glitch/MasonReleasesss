//
// Created by rei on 1/22/2026.
//

#include "core.hpp"

#include "assets/assets.hpp"
#include "fonts/fonts.hpp"
#include "graphics/graphics.hpp"
#include "ui/ui.hpp"
#include "ui/alerts/alerts.hpp"
#include "ui/binder/binder.hpp"
#include "ui/themes/themes.hpp"
#include "ui/widgets/widgets.hpp"

using everness::AliceKit;

using everness::CCore;

AliceKit::~AliceKit( ) = default;

AliceKit::AliceKit( )
{
    ui = std::make_unique< alice::CUserInterface >( );
    fonts = std::make_unique< alice::CFonts >( );
    alerts = std::make_unique< alice::CAlerts >( );
    binder = std::make_unique< alice::CBinder >( );
    themes = std::make_unique< alice::CThemes >( );
    widgets = std::make_unique< alice::CWidgets >( );
    assets = std::make_unique< alice::CAssets >( );
}

std::unique_ptr< CCore > everness::ctx;

CCore::~CCore( ) = default;

CCore::CCore( )
{
    alice = std::make_unique< AliceKit >( );
    graphics = std::make_unique< CGraphics >( );
}
