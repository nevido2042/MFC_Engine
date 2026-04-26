#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>

using Microsoft::WRL::ComPtr;

class CPicking
{
public:
    CPicking();
    ~CPicking();

    void Initialize(ID3D12Device* device, int width, int height);
    void Resize(ID3D12Device* device, int width, int height);
    
    // Reads a pixel from the picking render target and returns the decoded ID.
    // Must be called with an active command list.
    void ReadPixelAsync(ID3D12GraphicsCommandList* commandList, int x, int y);
    UINT GetPickedID();

    ID3D12Resource* GetRenderTarget() const { return m_pickingRT.Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const;

private:
    ComPtr<ID3D12Resource> m_pickingRT;
    ComPtr<ID3D12Resource> m_pickingDSV;
    ComPtr<ID3D12Resource> m_readbackBuffer;

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

    int m_width;
    int m_height;
};
