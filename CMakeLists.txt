cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0091 NEW)
project(MasonOverlay VERSION 1.0.0 LANGUAGES C CXX RC)
if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "Use Visual Studio 2022 on Windows with -A x64.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Use -A x64.")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# Preserve the original font backend; no network downloads/vcpkg.
# The unchanged imconfig.h selects stb_truetype for the UI font atlas.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
foreach(dep ZLIB BZIP2 PNG HARFBUZZ BROTLI)
    set(FT_DISABLE_${dep} ON CACHE BOOL "" FORCE)
endforeach()
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
add_subdirectory(product/backend/imgui/freetype EXCLUDE_FROM_ALL)
set_property(TARGET freetype PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_executable(MasonOverlay WIN32
    product/entry.cpp
    product/render/overlay.cpp
    product/render/menu.cpp
    product/render/configs.cpp
    product/render/kill_sound.cpp
    product/backend/imgui/imgui.cpp
    product/backend/imgui/imgui_demo.cpp
    product/backend/imgui/imgui_draw.cpp
    product/backend/imgui/imgui_tables.cpp
    product/backend/imgui/imgui_widgets.cpp
    product/backend/imgui/imgui_impl_win32.cpp
    product/backend/imgui/imgui_impl_dx11.cpp
    product/backend/imgui/imgui_freetype.cpp
    product/everness/src/alice/animator/animator.cpp
    product/everness/src/alice/assets/assets.cpp
    product/everness/src/alice/fonts/fonts.cpp
    product/everness/src/alice/ui/alerts/alerts.cpp
    product/everness/src/alice/ui/binder/binder.cpp
    product/everness/src/alice/ui/elem/elem.cpp
    product/everness/src/alice/ui/popups/popups.cpp
    product/everness/src/alice/ui/tabs/config/config.cpp
    product/everness/src/alice/ui/tabs/visual/visual.cpp
    product/everness/src/alice/ui/themes/themes.cpp
    product/everness/src/alice/ui/ui.cpp
    product/everness/src/alice/ui/widgets/hit_logs/hit_logs.cpp
    product/everness/src/alice/ui/widgets/keybinds/keybinds.cpp
    product/everness/src/alice/ui/widgets/watermark/watermark.cpp
    product/everness/src/alice/ui/widgets/widgets.cpp
    product/everness/src/config/base64.cpp
    product/everness/src/config/config.cpp
    product/everness/src/core.cpp
    product/everness/src/graphics/graphics.cpp
    assets/MasonOverlay.rc
)
target_include_directories(MasonOverlay PRIVATE product product/render product/backend/imgui product/everness/src product/everness/src/alice product/everness/thirdparty assets)
target_compile_definitions(MasonOverlay PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
target_compile_options(MasonOverlay PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/utf-8> $<$<COMPILE_LANGUAGE:CXX>:/bigobj> $<$<COMPILE_LANGUAGE:CXX>:/permissive-> $<$<COMPILE_LANGUAGE:CXX>:/W3>)
target_link_libraries(MasonOverlay PRIVATE freetype d3d11 dxgi d3dcompiler dwmapi user32 gdi32 imm32 shell32 advapi32)
target_link_options(MasonOverlay PRIVATE "/MANIFESTUAC:level='asInvoker' uiAccess='false'")
set_target_properties(MasonOverlay PROPERTIES OUTPUT_NAME MasonOverlay)
