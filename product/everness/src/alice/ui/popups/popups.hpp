//
// Created by Kai Tears on 25/01/2026.
//

#ifndef ENLIGHTENED_POPUPS_HPP
#define ENLIGHTENED_POPUPS_HPP

#include "animator/animator.hpp"
#include "imgui.h"
#include "popups.hpp"
#include "../binder/binder.hpp"

#include <functional>
#include <optional>
#include <string>
#include <variant>

namespace alice
{
    class CPopup;

    struct PopupEntries
    {
        static std::optional< float > itemHeightOverride;
        static std::optional< ImColor > textColorOverride;
        static std::optional< CPopup* > popupEnablerCallback;

        static void separator( );

        static bool button( const std::optional< std::string >& icon, const std::string& title );
        static bool toggle( const std::optional< std::variant< std::string, ImTextureID > >& icon, const std::string& title, bool* v );

        static bool sliderScalar( const std::optional< std::string >& icon, const std::string& title, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags );

        static bool sliderFloat( const std::optional< std::string >& icon, const std::string& title, float* v, float min, float max );
        static bool sliderInt( const std::optional< std::string >& icon, const std::string& title, int* v, int min, int max );

        static bool bindType( const std::optional< std::string >& icon, const std::string& title, BindType* bind );
        static bool bindListener( const std::optional< std::string >& icon, const std::string& title, ImGuiKey* bind, bool* listening );
    };

    class CPopup
    {
        std::string m_title;
        float m_width;

        ImGuiWindow* m_window { };

        ImVec2 m_mousePos { };
        CAnimator m_popupOpacity { float( 0.f ) };
        CAnimator m_popupBlur { float( 1.f ) };

        bool m_setFocus { false };

        std::optional< float > m_maxHeight { std::nullopt };
        std::optional< ImVec2 > m_forcePadding { std::nullopt };
        std::optional< ImVec2 > m_forcePos { std::nullopt };

        void applyBlur( ImGuiWindow* window );

    public:
        CPopup( float width, const std::string& title, const std::optional< ImVec2 >& force_pos = std::nullopt );
        ~CPopup( );

        bool opened { false };

        bool begin( bool handle = false );
        void end( );

        float getAnimationValue( );

        void setPadding( const ImVec2& padding );
        void setWidth( const float& width );
        void setMaxHeight( const float& height );
        void setPosition( const ImVec2& pos );
        void setTitle( const std::string& title );

        bool allowOpen( );
    };
} // namespace alice

#endif // ENLIGHTENED_POPUPS_HPP
