#pragma once

#include "data_management.hpp"

#include "imgui.h"
#include "ui/binder/binder.hpp"

namespace everness
{
    struct Config
    {
        declare_variable( bool, espRect, false );

        declare_variable( bool, uiEnableGlowLogo, true );
        declare_variable( bool, uiEnableGlowMain, true );

        std::vector< alice::BindBool > espRectBinds { };
        std::vector< alice::BindBool > teleportBinds { };
        std::vector< alice::BindBool > stopBinds { };
        std::vector< alice::BindBool > aimbotBinds { };
        std::vector< alice::BindBool > triggerbotBinds { };
        std::vector< alice::BindBool > autofireBinds { };
    };
} // namespace everness