#pragma once

#include <algorithm>
#include <bitset>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace everness
{
    class CBase64
    {
        static const char base64_encoding_table[ 64 ];
        static const int base64_decoding_table[ 131 ];

        static int binStringToDec( const std::string& binary_string );
        static bool isStringASCII( const std::string& message );

    public:
        static bool isNumber( const std::string& input );
        static std::string lineSplitting( const std::string& base64_message, int length );

        static std::string base64Encode( const std::string& message );
        static std::string base64Decode( std::string message );
    };
}