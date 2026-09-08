#pragma once
// Test double ONLY. Never install this file into product or backend.
#include <vector>
#include <cstring>
using ImU32=unsigned int;
struct ImVec2 { float x,y; ImVec2(float a=0,float b=0):x(a),y(b){} };
struct ImVec4 { float x,y,z,w; ImVec4(float a=0,float b=0,float c=0,float d=0):x(a),y(b),z(c),w(d){} };
template<class T> struct ImVector {
    std::vector<T> values; int Size=0;
    T& operator[](int i) {return values[i];}
    const T& operator[](int i) const {return values[i];}
    void push_back(const T& value) {values.push_back(value); Size=int(values.size());}
    void clear() {values.clear(); Size=0;}
    void erase_first() {values.erase(values.begin()); Size=int(values.size());}
};
enum {ImGuiMouseButton_COUNT=5, ImGuiCond_Always=1};
enum {ImGuiWindowFlags_ChildWindow=1, ImGuiWindowFlags_Popup=2, ImGuiWindowFlags_Tooltip=4};
struct ImGuiIO {
    bool WantSetMousePos=false;
    ImVec2 MousePos, MousePosPrev, MouseClickedPos[5];
    bool MouseDown[5]={};
    ImVec2 DisplaySize, DisplayFramebufferScale;
};
enum ImGuiInputEventType {ImGuiInputEventType_None, ImGuiInputEventType_MousePos, ImGuiInputEventType_MouseWheel};
struct ImGuiInputEvent {
    ImGuiInputEventType Type=ImGuiInputEventType_None;
    ImU32 EventId=0;
    struct {float PosX=0,PosY=0;} MousePos;
};
struct ImGuiWindow {const char* Name="test"; int Flags=0; ImVec2 Pos, Size;};
struct ImGuiContext {ImGuiIO IO; ImU32 ActiveId=0; ImVector<ImGuiWindow*> Windows; ImVector<ImGuiInputEvent> InputEventsQueue;};
struct ImDrawVert {ImVec2 pos,uv; ImU32 col=0;};
struct ImDrawCmd {ImVec4 ClipRect;};
struct ImDrawList {ImVector<ImDrawVert> VtxBuffer; ImVector<ImDrawCmd> CmdBuffer;};
struct ImDrawData {int CmdListsCount=0; ImDrawList** CmdLists=nullptr; ImVec2 DisplayPos,DisplaySize,FramebufferScale;};
namespace ImGui {
inline ImGuiContext test_context;
inline int test_frame=0;
inline ImGuiContext* GetCurrentContext() {return &test_context;}
inline ImGuiIO& GetIO() {return test_context.IO;}
inline int GetFrameCount() {return test_frame;}
inline void SetWindowPos(const char* name,ImVec2 pos,int) {
    for (int i=0;i<test_context.Windows.Size;++i)
        if(std::strcmp(test_context.Windows[i]->Name,name)==0) test_context.Windows[i]->Pos=pos;
}
}
