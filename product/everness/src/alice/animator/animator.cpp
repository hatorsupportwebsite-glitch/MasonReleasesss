//
// Created by rei on 1/17/2026.
//

#include "animator.hpp"

#include <cmath>
#include <algorithm>

using alice::CAnimator;

static float ease( float time, float start_value, float change_value, float duration )
{
    return change_value * time / duration + start_value;
}

float CAnimator::resolveValue( float clock, float previous_value, float new_value, float duration )
{
    float value = std::clamp(
        ImLerp( previous_value, new_value, ImGui::GetIO( ).DeltaTime * 12.f ),
        // ease( clock, previous_value, new_value - previous_value, duration ),
        std::min( previous_value, new_value ),
        std::max( previous_value, new_value ) );

    if ( std::abs( value - new_value ) < .001f )
        value = new_value;

    if ( std::fmod( value, 1 ) < .001f )
        value = std::floor( value );
    else if ( std::fmod( value, 1 ) > .99f )
        value = std::ceil( value );

    return value;
}

template < typename T, typename... Members >
void CAnimator::updateStruct( T& current, const T& target,
                              float duration, Members... members )
{
    auto resolve_comp = [ this, duration ]( auto& cur, auto tgt ) {
        cur = resolveValue( ImGui::GetIO( ).DeltaTime, cur, tgt, duration );
    };

    ( resolve_comp( current.*members, target.*members ), ... );
}

template < typename T >
void CAnimator::set( T to_value )
{
    value = to_value;
}

template < typename T >
void CAnimator::update( T to_value, float duration )
{
    if ( std::any_cast< T >( value ) == to_value )
        return;

    if constexpr ( std::is_arithmetic_v< T > )
    {
        value = resolveValue( ImGui::GetIO( ).DeltaTime,
                              std::any_cast< T >( value ),
                              to_value, duration );
    } else
    {
        T current = std::any_cast< T >( value );

        if constexpr ( requires { T::x; T::y; T::z; T::w; } )
        {
            updateStruct( current, to_value, duration,
                          &T::x, &T::y, &T::z, &T::w );
        } else if constexpr ( requires { T::x; T::y; T::z; } )
        {
            updateStruct( current, to_value, duration,
                          &T::x, &T::y, &T::z );
        } else if constexpr ( requires { T::x; T::y; } )
        {
            updateStruct( current, to_value, duration,
                          &T::x, &T::y );
        }

        value = current;
    }
}

template < typename T >
T CAnimator::get( ) const
{
    return std::any_cast< T >( value );
}

template CAnimator::CAnimator( float );
template CAnimator::CAnimator( ImVec4 );
template CAnimator::CAnimator( ImVec2 );

template void CAnimator::update< float >( float, float );
template void CAnimator::update< ImVec4 >( ImVec4, float );
template void CAnimator::update< ImVec2 >( ImVec2, float );

template void CAnimator::set< float >( float );
template void CAnimator::set< ImVec4 >( ImVec4 );
template void CAnimator::set< ImVec2 >( ImVec2 );

template float CAnimator::get< float >( ) const;
template ImVec4 CAnimator::get< ImVec4 >( ) const;
template ImVec2 CAnimator::get< ImVec2 >( ) const;