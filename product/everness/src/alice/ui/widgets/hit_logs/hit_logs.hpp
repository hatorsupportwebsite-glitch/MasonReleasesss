//
// Created by Kai Tears on 27/01/2026.
//

#ifndef ENLIGHTENED_HIT_LOGS_HPP
#define ENLIGHTENED_HIT_LOGS_HPP

#include "../widgets.hpp"
#include "animator/animator.hpp"

#include <chrono>
#include <vector>

namespace alice
{
    struct HitLog
    {
        std::string data;

        bool ended { false };
        std::chrono::steady_clock::time_point timestamp { };

        CAnimator alpha { float( 0.f ) };
        CAnimator y { float( 0.f ) };
        CAnimator win { float( 0.f ) };
    };

    class CHitLogs : public CBaseWidget
    {
        std::vector< HitLog > m_logs { };

        std::string stripColorTags( const std::string& data );
        ImVec2 calcWindowSize( );
        void renderColoredText( ImDrawList* draw, ImVec2 pos, const std::string& text, float alpha = 1.f );

    public:
        bool enabled = true;

        void pushLog( const std::string& data );
        void render( bool allow, bool demo = false );
    };
} // namespace alice

#endif // ENLIGHTENED_HIT_LOGS_HPP
