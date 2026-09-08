//
// Created by Kai Tears on 23/01/2026.
//

#ifndef ENLIGHTENED_UI_HPP
#define ENLIGHTENED_UI_HPP

#include "elem/elem.hpp"

#include <functional>
#include <optional>

namespace alice
{
    struct PushStyle
    {
        template < typename T >
        PushStyle( const ImGuiStyleVar& idx, const T& v )
        {
            ImGui::PushStyleVar( idx, v );
        }

        ~PushStyle( )
        {
            ImGui::PopStyleVar( );
        }
    };

    struct PushColor
    {
        PushColor( const ImGuiCol& idx, const ImColor& v )
        {
            ImGui::PushStyleColor( idx, v.Value );
        }

        ~PushColor( )
        {
            ImGui::PopStyleColor( );
        }
    };

    class CUserInterface
    {
        enum class TransitionTarget
        {
            None,
            Tab,
            SubTab
        };

        struct Transition
        {
            CAnimator alpha { 0.f };
            bool active { false };
            TransitionTarget target { TransitionTarget::None };

            std::function< void( ) > apply { nullptr };
        };

        enum class TabType
        {
            Category,
            Tab,
        };

        struct SubTab
        {
            std::string name;

            std::function< void( int ) > callback;

            CAnimator background { ImVec4( 0, 0, 0, 0 ) };
            CAnimator _name { ImVec4( 0, 0, 0, 0 ) };
        };

        struct Tab
        {
            std::string name;
            std::optional< std::string > icon;

            TabType type;

            std::function< void( int ) > callback;

            std::optional< std::vector< SubTab > > subTabs { std::nullopt };

            CAnimator background { ImVec4( 0, 0, 0, 0 ) };
            CAnimator _icon { ImVec4( 0, 0, 0, 0 ) };
            CAnimator _name { ImVec4( 0, 0, 0, 0 ) };
        };

        struct MenuSettings
        {
            CAnimator main { 0.f };
            bool presented { false };

            void render( const std::function< void( ) >& back_task );
        } m_settings;

        std::vector< Tab > m_tabs { };

        Transition m_transition { };

        int m_pendingTab { 0 };
        int m_pendingSubTab { 0 };

        int m_selectedSubTab { 0 };
        int m_selectedTab { 0 };

        std::function< void( int ) > m_callback { nullptr };

        CAnimator m_mainAreaInnerBlur { float( 0.f ) };
        CAnimator m_mainAreaOpacity { float( 0.f ) };
        CAnimator m_subTabsAreaOpacity { float( 0.f ) };

        CAnimator m_menuOpacity { float( 0.f ) };
        CAnimator m_overlayBlurIntensity { float( 1.f ) };

        bool m_presented { false };
        bool m_allowEdits { false };

        ImVec2 m_windowSize { 680, 460 };

        ImGuiWindow* m_window { nullptr };

        CUIElements m_elem;

        // TODO-ASAP: fix ts, move into ::split or something to avoid this
        static constexpr int kConfigsTab = 7;

        void applyBlurOverlay( ImGuiWindow* window ) const;

        void renderCategory( Tab* tab ) const;
        bool renderTab( Tab* tab, bool active, int width );
        bool renderSubTab( SubTab* subtab, bool active, int idx );

        void saveButton( const ImVec2& size, const std::function< void( ) >& save_fn );

    public:
        CUserInterface( );
        ~CUserInterface( );

        static ImTextureID logo;
        static ImTextureID profile_picture;

        void init( );
        void render( );

        bool& isPresented( );
    };
} // namespace alice

#endif // ENLIGHTENED_UI_HPP
