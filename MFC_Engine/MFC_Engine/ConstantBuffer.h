#pragma once
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class CConstantBuffer
{
public:
    CConstantBuffer();
    ~CConstantBuffer();

    void Initialize(ID3D12Device* device, UINT elementSize, UINT elementCount);
    void Update(UINT index, const void* pData, UINT dataSize);
    
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress(UINT index) const;
    ID3D12Resource* GetResource() const { return m_constantBuffer.Get(); }
    UINT8* GetMappedData() const { return m_pCbvDataBegin; }

private:
    ComPtr<ID3D12Resource> m_constantBuffer;
    UINT8* m_pCbvDataBegin;
    UINT m_elementSize;
    UINT m_elementCount;
};
