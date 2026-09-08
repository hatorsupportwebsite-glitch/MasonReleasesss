#include "mason_ui_dpi.hpp"
#include <cassert>
#include <iostream>

using namespace mason_ui;
using namespace mason_ui::dpi;
static bool near(float a,float b) { return std::abs(a-b)<0.001f; }
static void pixels() {ImGui::GetIO().DisplaySize=ImVec2(1920,1080);}
static ImGuiInputEvent mouse(ImU32 id,float x,float y) {
    ImGuiInputEvent event; event.Type=ImGuiInputEventType_MousePos; event.EventId=id;
    event.MousePos.PosX=x; event.MousePos.PosY=y; return event;
}
static void begin_frame() {
    before_platform_frame();
    pixels(); // Simulated Win32 NewFrame display size.
    before_new_frame();
}
int main() {
    // Uses explicit ImGui test doubles, not the Windows renderer or full UI.
    reset_options();
    frame=FrameState{};
    auto& g=*ImGui::GetCurrentContext();
    g=ImGuiContext{};
    g.IO.MousePos=ImVec2(400,200); g.IO.MousePosPrev=g.IO.MousePos;
    g.InputEventsQueue.push_back(mouse(1,400,200));
    begin_frame();
    assert(frame.scale==1.f && g.IO.DisplaySize.x==1920);
    assert(g.InputEventsQueue[0].MousePos.PosX==400);
    after_new_frame();
    g.InputEventsQueue.clear();

    ImGuiWindow hud; hud.Name="HUD"; hud.Pos=ImVec2(1600,950); hud.Size=ImVec2(180,80);
    ImGuiWindow child; child.Name="child"; child.Flags=ImGuiWindowFlags_ChildWindow; child.Pos=ImVec2(100,100);
    g.Windows.push_back(&hud); g.Windows.push_back(&child);
    options.dpi_percent=175;
    g.InputEventsQueue.push_back(mouse(2,875,350));
    g.InputEventsQueue.push_back(mouse(3,-FLT_MAX,-FLT_MAX));
    ImGuiInputEvent wheel; wheel.Type=ImGuiInputEventType_MouseWheel; wheel.EventId=4;
    wheel.MousePos.PosX=123; g.InputEventsQueue.push_back(wheel);
    begin_frame();
    assert(near(frame.scale,1.75f));
    assert(near(g.IO.DisplaySize.x,1920.f/1.75f));
    assert(near(g.InputEventsQueue[0].MousePos.PosX,500.f));
    assert(near(g.InputEventsQueue[0].MousePos.PosY,200.f));
    assert(g.InputEventsQueue[1].MousePos.PosX==-FLT_MAX);
    assert(g.InputEventsQueue[2].MousePos.PosX==123);
    assert(hud.Pos.x+hud.Size.x<=g.IO.DisplaySize.x+0.001f);
    assert(hud.Pos.y+hud.Size.y<=g.IO.DisplaySize.y+0.001f);
    assert(child.Pos.x==100 && child.Pos.y==100);
    after_new_frame();
    assert(g.InputEventsQueue[0].MousePos.PosX==875);
    assert(g.InputEventsQueue[0].MousePos.PosY==350);
    // An unconsumed/trickled event remains in physical coordinates for next frame.
    begin_frame();
    assert(near(g.InputEventsQueue[0].MousePos.PosX,500));
    g.InputEventsQueue.erase_first(); // Simulated consumption by NewFrame.
    after_new_frame();
    assert(g.InputEventsQueue[0].MousePos.PosX==-FLT_MAX);
    g.InputEventsQueue.clear();

    options.dpi_percent=200;
    g.ActiveId=42;
    begin_frame(); assert(near(frame.scale,1.75f)); after_new_frame();
    g.ActiveId=0; g.IO.MouseDown[0]=true;
    begin_frame(); assert(near(frame.scale,1.75f)); after_new_frame();
    g.IO.MouseDown[0]=false;
    begin_frame(); assert(frame.scale==2.f); after_new_frame();

    ImDrawList menu, keybinds, watermark, hitlogs;
    ImDrawList* lists[]={&menu,&keybinds,&watermark,&hitlogs};
    for(int i=0;i<4;++i) {
        ImDrawVert vertex; vertex.pos=ImVec2(10.f+i,20.f+i); vertex.uv=ImVec2(.2f,.4f); vertex.col=0xff123456;
        lists[i]->VtxBuffer.push_back(vertex);
        ImDrawCmd command; command.ClipRect=ImVec4(1,2,30,40); lists[i]->CmdBuffer.push_back(command);
    }
    ImDrawData data; data.CmdListsCount=4; data.CmdLists=lists;
    data.DisplaySize=ImVec2(960,540); data.FramebufferScale=ImVec2(1,1);
    ++ImGui::test_frame;
    scale_draw_data(&data);
    for(int i=0;i<4;++i) {
        assert(lists[i]->VtxBuffer[0].pos.x==(10.f+i)*2);
        assert(lists[i]->VtxBuffer[0].pos.y==(20.f+i)*2);
        assert(lists[i]->VtxBuffer[0].uv.x==.2f && lists[i]->VtxBuffer[0].col==0xff123456);
        assert(lists[i]->CmdBuffer[0].ClipRect.z==60);
    }
    assert(data.DisplaySize.x==1920 && data.DisplaySize.y==1080);
    scale_draw_data(&data); // Double render in one frame must not multiply again.
    assert(menu.VtxBuffer[0].pos.x==20);
    scale_draw_data(nullptr);

    g.IO.WantSetMousePos=true; g.IO.MousePos=ImVec2(100,80);
    before_platform_frame();
    assert(g.IO.MousePos.x==200 && g.IO.MousePos.y==160);
    pixels(); before_new_frame();
    assert(g.IO.MousePos.x==100 && g.IO.MousePos.y==80);
    after_new_frame(); g.IO.WantSetMousePos=false;

    options.dpi_percent=999;
    begin_frame(); assert(frame.scale==2.f); after_new_frame();
    options.dpi_percent=1;
    begin_frame(); assert(frame.scale==.75f); after_new_frame();

    std::cout << "PASS: DPI input conversion, exact queued-event restoration, delayed scale changes, all four draw lists, clip rectangles, cursor warps, bounds and no double scaling.\n";
    std::cout << "LIMITATION: tests use ImGui doubles; they are not a full Windows visual or integration test.\n";
}
