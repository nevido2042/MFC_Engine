#pragma once
#include <wrl.h>
#include <memory>
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

class CPickingSystem
{
public:
    static CPickingSystem& GetInstance()
    {
        static CPickingSystem instance;
        return instance;
    }

    void Initialize(ComPtr<ID3D12Device> device, ID3D12RootSignature* rootSignature, int width, int height);
    void Resize(ComPtr<ID3D12Device> device, int width, int height);
    
    UINT Pick(int x, int y, class CGraphicsEngine* pEngine);

private:
    void CreateReadbackBuffer(ID3D12Device* device);

private:
    ComPtr<ID3D12Resource> m_pReadbackBuffer;
    int m_nWidth;
    int m_nHeight;
};
