#include <windows.h>
#include <string_view>
#include <exception>
#include "render/overlay.h"
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR command_line,int) {
    try {
        const std::wstring_view args=command_line?command_line:L"";
        return mason::RunOverlay(args.find(L"--smoke-test")!=std::wstring_view::npos);
    } catch(const std::exception& error) {
        MessageBoxA(nullptr,error.what(),"Mason",MB_OK|MB_ICONERROR);return 1;
    }
}
