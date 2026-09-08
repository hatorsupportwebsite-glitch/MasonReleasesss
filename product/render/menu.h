#pragma once
struct ID3D11Device;
struct ID3D11DeviceContext;
namespace mason {
bool InitializeMenu(ID3D11Device*,ID3D11DeviceContext*);
void RenderMenu();
void ShutdownMenu();
bool IsMenuOpen();
void SetMenuOpen(bool);
}
