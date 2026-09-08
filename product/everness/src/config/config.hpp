//
// Created by rei on 12/8/2025.
//

#ifndef EVERNESS_SO2_EXTERNAL_CONFIG_HPP
#define EVERNESS_SO2_EXTERNAL_CONFIG_HPP

#include "fnv1a.hpp"

#include <string>

namespace everness
{
    class CConfig
    {
        std::size_t getVariableIdx( fnv1a_t m_name_hash );

    public:
        std::string get( );
        void load( const std::string& b64 );
    };
} // namespace everness

#endif // EVERNESS_SO2_EXTERNAL_CONFIG_HPP
