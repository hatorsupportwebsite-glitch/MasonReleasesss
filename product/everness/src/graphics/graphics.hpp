#pragma once
#include <cstddef>
#include <vector>
#include "imgui.h"
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
namespace everness {
class CGraphics {
    std::vector<ID3D11ShaderResourceView*> textures;
public:
    static ID3D11Device* m_device;
    static ID3D11DeviceContext* m_deviceContext;
    ~CGraphics();
    ImTextureID loadTextureFromFile(const char* file_name);
    ImTextureID loadTextureFromMemory(const void* data, std::size_t size, bool neutral_rgb=false);
};
}
