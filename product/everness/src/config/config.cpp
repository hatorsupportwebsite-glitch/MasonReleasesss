//
// Created by rei on 12/8/2025.
//

#include "config.hpp"

#include "core.hpp"

#include "json/json.hpp"
#include "data.hpp"
#include "base64.hpp"

using everness::CConfig;

std::size_t CConfig::getVariableIdx( const fnv1a_t m_name_hash )
{
    for ( size_t i = 0U; i < c::variables.size( ); i++ )
    {
        if ( c::variables.at( i ).m_name_hash == m_name_hash )
            return i;
    }

    return static_cast< std::size_t >( -1 );
}

void packVector2( nlohmann::json& config, const ImVec2& value )
{
    config[ ( "x" ) ] = value.x;
    config[ ( "y" ) ] = value.y;
}

ImVec2 getVector2( nlohmann::json config )
{
    float x, y;

    x = config[ ( "x" ) ].get< float >( );
    y = config[ ( "y" ) ].get< float >( );

    return { x, y };
}

void packVector4( nlohmann::json& config, const ImVec4& value )
{
    config[ ( "x" ) ] = value.x;
    config[ ( "y" ) ] = value.y;
    config[ ( "z" ) ] = value.z;
    config[ ( "w" ) ] = value.w;
}

ImVec4 getVector4( nlohmann::json config )
{
    float r, g, b, a;

    r = config[ ( "x" ) ].get< float >( );
    g = config[ ( "y" ) ].get< float >( );
    b = config[ ( "z" ) ].get< float >( );
    a = config[ ( "w" ) ].get< float >( );

    return { r, g, b, a };
}

std::string CConfig::get( )
{
    nlohmann::json settings { };

    try
    {
        nlohmann::json variables_json = nlohmann::json::array( );

        for ( auto& variable : c::variables )
        {
            nlohmann::json entry = { };

            entry[ ( "name-id" ) ] = variable.m_name_hash;
            entry[ ( "type-id" ) ] = variable.m_type_hash;

            switch ( variable.m_type_hash )
            {
            case fnv1a::hash_const( "int" ):
            {
                entry[ ( "value" ) ] = variable.get< int >( );

                break;
            }
            case fnv1a::hash_const( "float" ):
            {
                entry[ ( "value" ) ] = variable.get< float >( );

                break;
            }
            case fnv1a::hash_const( "bool" ):
            {
                entry[ ( "value" ) ] = variable.get< bool >( );

                break;
            }
            case fnv1a::hash_const( "std::string" ):
            {
                entry[ ( "value" ) ] = variable.get< std::string >( );

                break;
            }
            case fnv1a::hash_const( "ImVec4" ):
            {
                packVector4( entry[ ( "value" ) ], variable.get< ImVec4 >( ) );

                break;
            }
            case fnv1a::hash_const( "ImVec2" ):
            {
                packVector2( entry[ ( "value" ) ], variable.get< ImVec2 >( ) );

                break;
            }
            default:
                break;
            }

            variables_json.push_back( entry );
        }

        settings[ ( "variables" ) ] = variables_json;
    } catch ( const nlohmann::detail::exception& ex )
    {
        return "";
    }

    return CBase64::base64Encode( settings.dump( 4 ) );
}

void CConfig::load( const std::string& b64 )
{
    if ( strlen( b64.c_str( ) ) < 10 )
        return;

    std::string const input = CBase64::base64Decode( b64 );

    std::string decoded;
    try
    {
        decoded = CBase64::base64Decode( b64 );
    } catch ( ... )
    {
        return;
    }

    nlohmann::json settings;
    try
    {
        settings = nlohmann::json::parse( decoded );
    } catch ( ... )
    {
        return;
    }

    if ( settings.is_discarded( ) )
        return;

    if ( !settings.contains( ( "variables" ) ) ||
         !settings[ ( "variables" ) ].is_array( ) )
    {
        return;
    }

    try
    {
        for ( const nlohmann::json& variable : settings[ ( "variables" ) ] )
        {
            if ( !variable.is_object( ) )
                continue;

            if ( !variable.contains( ( "name-id" ) ) ||
                 !variable.contains( ( "type-id" ) ) ||
                 !variable.contains( ( "value" ) ) )
            {
                continue;
            }

            fnv1a_t name_hash = 0;
            fnv1a_t type_hash = 0;

            try
            {
                name_hash = variable[ ( "name-id" ) ].get< fnv1a_t >( );
                type_hash = variable[ ( "type-id" ) ].get< fnv1a_t >( );
            } catch ( ... )
            {
                continue;
            }

            std::size_t const idx = getVariableIdx( name_hash );
            if ( idx == static_cast< std::size_t >( -1 ) )
                continue;

            auto& entry = c::variables[ idx ];

            try
            {
                switch ( type_hash )
                {
                case fnv1a::hash_const( "bool" ):
                    if ( variable[ ( "value" ) ].is_boolean( ) )
                        entry.set< bool >( variable[ ( "value" ) ].get< bool >( ) );
                    break;

                case fnv1a::hash_const( "float" ):
                    if ( variable[ ( "value" ) ].is_number( ) )
                        entry.set< float >( variable[ ( "value" ) ].get< float >( ) );
                    break;

                case fnv1a::hash_const( "int" ):
                    if ( variable[ ( "value" ) ].is_number_integer( ) )
                        entry.set< int >( variable[ ( "value" ) ].get< int >( ) );
                    break;

                case fnv1a::hash_const( "std::string" ):
                    if ( variable[ ( "value" ) ].is_string( ) )
                        entry.set< std::string >( variable[ ( "value" ) ].get< std::string >( ) );
                    break;

                case fnv1a::hash_const( "ImVec4" ):
                {
                    entry.set< ImVec4 >( getVector4( variable[ ( "value" ) ] ) );

                    break;
                }
                case fnv1a::hash_const( "ImVec2" ):
                {
                    entry.set< ImVec2 >( getVector2( variable[ ( "value" ) ] ) );

                    break;
                }

                default:
                    break;
                }
            } catch ( ... )
            {
                continue;
            }
        }
    } catch ( const nlohmann::detail::exception& ex )
    {
        return;
    }
}