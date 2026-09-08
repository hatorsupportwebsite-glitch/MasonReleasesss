//
// Created by Kai Tears on 23/01/2026.
//

#ifndef ENLIGHTENED_WATERMARK_HPP
#define ENLIGHTENED_WATERMARK_HPP

#include "imgui.h"
#include "imgui_internal.h"
#include "../widgets.hpp"

namespace alice
{
    class CWatermark : public CBaseWidget
    {
    public:
        bool enabled = true;

        void render( bool allow );
    };
} // namespace alice

#endif // ENLIGHTENED_WATERMARK_HPP
