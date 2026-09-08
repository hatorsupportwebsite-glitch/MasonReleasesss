#include "overlay.h"
#include "menu.h"
#include <windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND,UINT,WPARAM,LPARAM);
namespace {
template<class T> void release(T*& p){if(p){p->Release();p=nullptr;}}
struct DesktopOverlay {
    HWND window=nullptr;
    ID3D11Device* device=nullptr;
    ID3D11DeviceContext* context=nullptr;
    IDXGISwapChain* swap=nullptr;
    ID3D11RenderTargetView* target=nullptr;
    UINT width=0,height=0;
    bool display_changed=false;
    bool create_target(){
        ID3D11Texture2D* back=nullptr;
        if(FAILED(swap->GetBuffer(0,IID_PPV_ARGS(&back))))return false;
        auto hr=device->CreateRenderTargetView(back,nullptr,&target);back->Release();
        return SUCCEEDED(hr)&&target;
    }
    bool create_device(bool software){
        DXGI_SWAP_CHAIN_DESC desc{};
        desc.BufferCount=2;desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;desc.OutputWindow=window;
        desc.SampleDesc.Count=1;desc.Windowed=TRUE;desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
        const D3D_FEATURE_LEVEL levels[]={D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_0};
        D3D_FEATURE_LEVEL level{};
        auto create=[&](D3D_DRIVER_TYPE type){return D3D11CreateDeviceAndSwapChain(nullptr,type,nullptr,0,levels,2,D3D11_SDK_VERSION,&desc,&swap,&device,&level,&context);};
        auto hr=create(software?D3D_DRIVER_TYPE_WARP:D3D_DRIVER_TYPE_HARDWARE);
        if(FAILED(hr)&&!software){release(swap);release(context);release(device);hr=create(D3D_DRIVER_TYPE_WARP);}
        return SUCCEEDED(hr)&&create_target();
    }
    ~DesktopOverlay(){
        if(context){context->ClearState();context->Flush();}
        release(target);release(swap);release(context);release(device);
    }
};
DesktopOverlay* app_ptr=nullptr;
LRESULT CALLBACK wndproc(HWND w,UINT m,WPARAM a,LPARAM b){
    if(ImGui::GetCurrentContext()&&ImGui_ImplWin32_WndProcHandler(w,m,a,b))return 1;
    switch(m){
    case WM_SIZE:if(app_ptr&&a!=SIZE_MINIMIZED){app_ptr->width=LOWORD(b);app_ptr->height=HIWORD(b);}return 0;
    case WM_DISPLAYCHANGE:if(app_ptr)app_ptr->display_changed=true;return 0;
    case WM_SYSCOMMAND:if((a&0xfff0)==SC_KEYMENU)return 0;break;
    case WM_DESTROY:PostQuitMessage(0);return 0;
    }
    return DefWindowProcW(w,m,a,b);
}
bool pressed(int key,bool& was){bool down=(GetAsyncKeyState(key)&0x8000)!=0;bool edge=down&&!was;was=down;return edge;}
}
int mason::RunOverlay(bool smoke_test){
    ImGui_ImplWin32_EnableDpiAwareness();
    HINSTANCE instance=GetModuleHandleW(nullptr);
    HWND previous_foreground=GetForegroundWindow();
    WNDCLASSEXW wc{};
    wc.cbSize=sizeof(wc);wc.style=CS_HREDRAW|CS_VREDRAW;wc.lpfnWndProc=wndproc;
    wc.hInstance=instance;wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);
    wc.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(1));wc.hIconSm=wc.hIcon;
    wc.lpszClassName=L"MasonStandaloneOverlay";
    if(!RegisterClassExW(&wc))return 1;
    DesktopOverlay app;app_ptr=&app;
    app.window=CreateWindowExW(WS_EX_TOPMOST|WS_EX_LAYERED|WS_EX_TOOLWINDOW,wc.lpszClassName,L"Mason",WS_POPUP,
        0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),nullptr,nullptr,instance,nullptr);
    if(!app.window){app_ptr=nullptr;UnregisterClassW(wc.lpszClassName,instance);return 2;}
    const MARGINS margins={-1,-1,-1,-1};DwmExtendFrameIntoClientArea(app.window,&margins);
    SetLayeredWindowAttributes(app.window,0,255,LWA_ALPHA);
    int result=0;bool imgui=false,win32=false,dx11=false,menu=false;
    if(!app.create_device(smoke_test))result=3;
    else{
        IMGUI_CHECKVERSION();ImGui::CreateContext();imgui=true;
        auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.LogFilename=nullptr;
        io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;ImGui::StyleColorsDark();
        win32=ImGui_ImplWin32_Init(app.window);dx11=ImGui_ImplDX11_Init(app.device,app.context);
        if(!win32||!dx11)result=4;
        else{menu=true;if(!InitializeMenu(app.device,app.context))result=5;
            else if(!io.Fonts->Build()||!ImGui_ImplDX11_CreateDeviceObjects())result=6;}
    }
    if(result==0){
        ShowWindow(app.window,smoke_test?SW_SHOWNOACTIVATE:SW_SHOWDEFAULT);UpdateWindow(app.window);
        if(!smoke_test)SetForegroundWindow(app.window);
        bool done=false,applied_open=true;
        bool insert_down=(GetAsyncKeyState(VK_INSERT)&0x8000)!=0,end_down=(GetAsyncKeyState(VK_END)&0x8000)!=0;
        unsigned frames=0;
        while(!done){
            MSG msg{};
            while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);if(msg.message==WM_QUIT)done=true;}
            if(done)break;
            if(!smoke_test){if(pressed(VK_END,end_down))break;if(pressed(VK_INSERT,insert_down))SetMenuOpen(!IsMenuOpen());}
            const bool open=IsMenuOpen();
            if(open!=applied_open){
                LONG_PTR style=WS_EX_TOPMOST|WS_EX_LAYERED|WS_EX_TOOLWINDOW;
                if(!open)style|=WS_EX_TRANSPARENT|WS_EX_NOACTIVATE;
                SetWindowLongPtrW(app.window,GWL_EXSTYLE,style);
                SetWindowPos(app.window,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_FRAMECHANGED);
                if(open){HWND fg=GetForegroundWindow();if(fg!=app.window)previous_foreground=fg;SetForegroundWindow(app.window);}
                else if(GetForegroundWindow()==app.window&&IsWindow(previous_foreground))SetForegroundWindow(previous_foreground);
                applied_open=open;
            }
            if(app.display_changed){SetWindowPos(app.window,HWND_TOPMOST,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),SWP_NOACTIVATE);app.display_changed=false;}
            if(app.width&&app.height){
                app.context->OMSetRenderTargets(0,nullptr,nullptr);release(app.target);
                auto hr=app.swap->ResizeBuffers(0,app.width,app.height,DXGI_FORMAT_UNKNOWN,0);app.width=app.height=0;
                if(FAILED(hr)||!app.create_target()){result=7;break;}
            }
            ImGui_ImplDX11_NewFrame();ImGui_ImplWin32_NewFrame();ImGui::NewFrame();
            RenderMenu();ImGui::Render();
            const float clear[4]={0,0,0,0};app.context->OMSetRenderTargets(1,&app.target,nullptr);
            app.context->ClearRenderTargetView(app.target,clear);ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            auto hr=app.swap->Present(smoke_test?0:1,0);
            if(FAILED(hr)){result=8;break;}if(hr==DXGI_STATUS_OCCLUDED)Sleep(50);
            if(smoke_test&&++frames>=30)break;
        }
    }
    if(menu)ShutdownMenu();if(dx11)ImGui_ImplDX11_Shutdown();if(win32)ImGui_ImplWin32_Shutdown();
    if(imgui)ImGui::DestroyContext();if(IsWindow(app.window))DestroyWindow(app.window);
    app_ptr=nullptr;UnregisterClassW(wc.lpszClassName,instance);
    if(result&&!smoke_test)MessageBoxW(nullptr,L"Could not render the Mason desktop overlay.\nWindows 10/11 with Direct3D 11 is required.",L"Mason",MB_OK|MB_ICONERROR);
    return result;
}
