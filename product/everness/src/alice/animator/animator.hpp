//
// Created by rei on 1/17/2026.
//

#ifndef DREAMCHASERS_LOADER_ANIMATOR_H
#define DREAMCHASERS_LOADER_ANIMATOR_H

#include <any>
#include <memory>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"

namespace alice
{
    class CAnimator
    {
    public:
        template < typename T >
        CAnimator( T initial_value = T { } )
            : value( initial_value ) { };

        template < typename T >
        void update( T to_value, float duration = 0.10f );

        template < typename T >
        [[nodiscard]] T get( ) const;

        template < typename T >
        void set( T to_value );

        CAnimator( ) = default;
        ~CAnimator( ) = default;

        template < typename T >
        static CAnimator instantiate( T )
        {
            return c_animator( T { } );
        }

        std::any value;

    private:
        static float resolveValue( float clock, float previous_value, float new_value, float duration );

        template < typename T, typename... Members >
        void updateStruct( T& current, const T& target,
                           float duration, Members... members );
    };

    struct Color
    {
        ImVec4 value;

        constexpr Color( ) {}
        constexpr Color( float r, float g, float b, float a = 1.0f )
            : value( r, g, b, a )
        {
        }
        constexpr Color( const ImVec4& col )
            : value( col )
        {
        }
        constexpr Color( const ImColor& col )
            : value( col.Value )
        {
        }
        constexpr Color( int r, int g, int b, int a = 255 )
            : value( ( float ) r * ( 1.0f / 255.0f ), ( float ) g * ( 1.0f / 255.0f ), ( float ) b * ( 1.0f / 255.0f ), ( float ) a * ( 1.0f / 255.0f ) )
        {
        }
        constexpr Color( ImU32 rgba )
            : value( ( float ) ( ( rgba >> IM_COL32_R_SHIFT ) & 0xFF ) * ( 1.0f / 255.0f ), ( float ) ( ( rgba >> IM_COL32_G_SHIFT ) & 0xFF ) * ( 1.0f / 255.0f ), ( float ) ( ( rgba >> IM_COL32_B_SHIFT ) & 0xFF ) * ( 1.0f / 255.0f ), ( float ) ( ( rgba >> IM_COL32_A_SHIFT ) & 0xFF ) * ( 1.0f / 255.0f ) )
        {
        }

        Color dim( float dim ) const
        {
            return { value.x * dim, value.y * dim, value.z * dim, value.w };
        }

        ImColor modulate( float alpha = ImGui::GetStyle( ).Alpha ) const
        {
            return { value.x, value.y, value.z, value.w * alpha };
        }

        ImColor u32( ) const
        {
            return value;
        }
    };
} // namespace alice

#endif // DREAMCHASERS_LOADER_ANIMATOR_H