//
// Created by rei on 1/22/2026.
//

#ifndef EVERNESSEU_CORE_HPP
#define EVERNESSEU_CORE_HPP

#include <memory>
#include <string>

#include "config/data.hpp"

namespace alice
{
    class CUserInterface;
    class CFonts;
    class CAlerts;
    class CBinder;
    class CThemes;
    class CWidgets;
    class CAssets;
} // namespace alice

namespace everness
{
    class CGraphics;

    struct AliceKit
    {
        AliceKit( );
        ~AliceKit( );

        std::unique_ptr< alice::CUserInterface > ui;
        std::unique_ptr< alice::CFonts > fonts;
        std::unique_ptr< alice::CAlerts > alerts;
        std::unique_ptr< alice::CBinder > binder;
        std::unique_ptr< alice::CThemes > themes;
        std::unique_ptr< alice::CWidgets > widgets;
        std::unique_ptr< alice::CAssets > assets;

        int resizeWidth;
        int resizeHeight;
    };

    class CCore
    {
    public:
        CCore( );
        ~CCore( );

        std::unique_ptr< AliceKit > alice;

        std::unique_ptr< CGraphics > graphics;

        Config config;

        bool safeMode { true };
    };

    extern std::unique_ptr< CCore > ctx;
} // namespace everness

#endif // EVERNESSEU_CORE_HPP
