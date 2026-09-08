//
// Created by Kai Tears on 23/01/2026.
//

#include "widgets.hpp"

#include "core.hpp"
#include "animator/animator.hpp"
#include "ui/ui.hpp"

#include "watermark/watermark.hpp"
#include "keybinds/keybinds.hpp"
#include "hit_logs/hit_logs.hpp"

using alice::CBaseWidget;
using alice::CWidgets;

void CBaseWidget::renderSelectionOutline( const ImRect& rect, float alpha, float rounding )
{
    auto* draw = ImGui::GetBackgroundDrawList( );

    draw->AddRect( rect.Min - ImVec2( 3, 3 ), rect.Max + ImVec2( 3, 3 ), Color( 255, 255, 255 ).modulate( alpha ), rounding );
}

CWidgets::CWidgets( )
{
    watermark = std::make_unique< CWatermark >( );
    hitLogs = std::make_unique< CHitLogs >( );
    keybinds = std::make_unique< CKeybinds >( );
}

CWidgets::~CWidgets( ) = default;

void CWidgets::render( )
{
    auto const presented = everness::ctx->alice->ui->isPresented( );

    hitLogs->render( presented, presented );
    watermark->render( presented );

    auto cleanup = []( auto& vec ) {
        std::erase_if( vec, []( auto& bind ) { return bind.removed && bind.alpha.template get< float >( ) < 0.001f; } );
    };
    cleanup( everness::ctx->config.espRectBinds );
    cleanup( everness::ctx->config.teleportBinds );
    cleanup( everness::ctx->config.stopBinds );
    cleanup( everness::ctx->config.aimbotBinds );
    cleanup( everness::ctx->config.triggerbotBinds );
    cleanup( everness::ctx->config.autofireBinds );

    keybinds->render( presented, []( ) -> std::vector< KeyBindEntry > {
        std::vector< KeyBindEntry > result { };

        auto process = [ &result ]( auto& vec ) {
            for ( auto& b : vec )
            {
                result.push_back( KeyBindEntry { b.name, std::nullopt, b.target && ( *b.target ) && b.visible, b.removed } );
                everness::ctx->alice->binder->handleBool( b );
            }
        };

        process( everness::ctx->config.espRectBinds );
        process( everness::ctx->config.teleportBinds );
        process( everness::ctx->config.stopBinds );
        process( everness::ctx->config.aimbotBinds );
        process( everness::ctx->config.triggerbotBinds );
        process( everness::ctx->config.autofireBinds );

        return result;
    }( ) );
}