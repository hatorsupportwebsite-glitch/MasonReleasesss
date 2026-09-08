#include "graphics.hpp"
#include <d3d11.h>
#include <fstream>
#include <limits>
#include <climits>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.hpp"
namespace everness {
ID3D11Device* CGraphics::m_device=nullptr;
ID3D11DeviceContext* CGraphics::m_deviceContext=nullptr;
CGraphics::~CGraphics() { for(auto* t:textures) if(t) t->Release(); }
ImTextureID CGraphics::loadTextureFromMemory(const void* data,std::size_t size,bool neutral_rgb) {
    if(!m_device || !m_deviceContext || !data || !size || size>INT_MAX) return 0;
    int w=0,h=0,channels=0;
    auto* pixels=stbi_load_from_memory(static_cast<const unsigned char*>(data),static_cast<int>(size),&w,&h,&channels,4);
    if(!pixels) return 0;
    if(w<=0 || h<=0 || w>8192 || h>8192) { stbi_image_free(pixels); return 0; }
    // Preserve the supplied alpha channel and padding. White RGB accepts the theme tint.
    if(neutral_rgb)
        for(std::size_t i=0,n=std::size_t(w)*h;i<n;++i)
            pixels[4*i]=pixels[4*i+1]=pixels[4*i+2]=255;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width=w;desc.Height=h;desc.MipLevels=0;desc.ArraySize=1;
    desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.SampleDesc.Count=1;
    desc.Usage=D3D11_USAGE_DEFAULT;
    desc.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;
    desc.MiscFlags=D3D11_RESOURCE_MISC_GENERATE_MIPS;
    ID3D11Texture2D* texture=nullptr;
    if(FAILED(m_device->CreateTexture2D(&desc,nullptr,&texture))) { stbi_image_free(pixels);return 0; }
    m_deviceContext->UpdateSubresource(texture,0,nullptr,pixels,UINT(w*4),0);
    stbi_image_free(pixels);
    D3D11_SHADER_RESOURCE_VIEW_DESC view{};
    view.Format=desc.Format;view.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;view.Texture2D.MipLevels=UINT(-1);
    ID3D11ShaderResourceView* srv=nullptr;
    const HRESULT hr=m_device->CreateShaderResourceView(texture,&view,&srv);texture->Release();
    if(FAILED(hr)||!srv) return 0;
    m_deviceContext->GenerateMips(srv);textures.push_back(srv);
    return (ImTextureID)srv;
}
ImTextureID CGraphics::loadTextureFromFile(const char* name) {
    if(!name) return 0;
    std::ifstream f(name,std::ios::binary|std::ios::ate);if(!f)return 0;
    auto n=f.tellg();if(n<=0||n>32*1024*1024)return 0;
    std::vector<unsigned char> bytes(static_cast<std::size_t>(n));f.seekg(0);
    if(!f.read(reinterpret_cast<char*>(bytes.data()),n))return 0;
    return loadTextureFromMemory(bytes.data(),bytes.size());
}
}
