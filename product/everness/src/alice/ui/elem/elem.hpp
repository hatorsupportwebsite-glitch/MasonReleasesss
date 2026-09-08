//
// Created by Kai Tears on 24/01/2026.
//

#ifndef ENLIGHTENED_ELEM_HPP
#define ENLIGHTENED_ELEM_HPP

#include "animator/animator.hpp"

#include <functional>
#include <optional>

namespace alice
{
    class CPopup;

    enum class ElemType
    {
        Single,
        Begin,
        Middle,
        End
    };

    class CUIElements
    {
        static constexpr int kElemHeight = 34;
        static ImVec4 gColorClipboard;
        static std::vector< ImVec4 > gColorPallette;

        static float adjHeight( const ElemType& type, const float& input );
        static void renderBackground( ImDrawList* draw, const ElemType& type, const ImRect& rect, const ImColor& background, const ImColor& outline );
        static void renderColorPicker( ImVec4* color );

        static bool unsafe;

    public:
        static void pushUnsafe( );
        static void popUnsafe( );

        static void applyLiquidGlass( const std::function< void( ImDrawList* ) >& base_render, ImDrawList* draw, const ImRect& rect, const ImColor& base_color, const ImColor& highlight_color );
        static void drawShadowRect( ImDrawList* draw, const ImVec2& obj_min, const ImVec2& obj_max, ImU32 shadow_col, float shadow_thickness, const ImVec2& shadow_offset, ImDrawFlags flags = 0, float obj_rounding = 0.0f );

        static void text( const std::string& text, bool bottom = false );

        static bool button( const ElemType& type, const std::string& label );

        static void color( const ElemType& type, const std::string& label, const std::vector< ImVec4* >& colors );

        static bool toggle( const ElemType& type, const std::string& label, bool* v, const std::vector< ImVec4* >& colors = { }, const std::optional< std::pair< CPopup*, std::function< void( ) > > >& popup = std::nullopt );

        static bool combo( const ElemType& type, const char* label, int* value, const std::vector< std::string >& entries );
        static bool combo( const ElemType& type, const char* label, int* value, const std::vector< std::pair< ImTextureID, std::string > >& entries );
        static void multiCombo( const ElemType& type, const std::string& label, const std::vector< std::pair< bool*, std::string > >& entries );

        static bool beginDropdown( const ElemType& type, CPopup* popup, const char* label, const char* preview_value, int items_count );

        static void config( const std::string& name, const int& id, const ImVec2& size_arg, bool selected, const std::function< void( ) > delete_fn, const std::function< void( ) > load_fn, const std::function< void( ) > update_fn );
    };
} // namespace alice

#endif // ENLIGHTENED_ELEM_HPP
