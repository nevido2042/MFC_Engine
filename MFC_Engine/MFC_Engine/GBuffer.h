#pragma once
#include "pch.h"

class CDevice;

class CGBuffer
{
public:
    CGBuffer();
    ~CGBuffer();

    void Initialize(CDevice* pDevice, int width, int height);
    void Resize(CDevice* pDevice, int width, int height);

    void TransitionToRenderTarget(ID3D12GraphicsCommandList* pCmdList);
    void TransitionToShaderResource(ID3D12GraphicsCommandList* pCmdList);
    void ClearAndSet(ID3D12GraphicsCommandList* pCmdList);

    ID3D12DescriptorHeap* GetSrvHeap() const { return m_pSrvHeap.Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandle() const;

private:
    void CreateHeaps(CDevice* pDevice);
    void CreateResources(CDevice* pDevice, int width, int height);
    void ClearResources();

private:
    static const UINT BufferCount = 3; // Position, Normal, Albedo

    ComPtr<ID3D12Resource> m_pGBuffers[BufferCount];
    ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_pSrvHeap;

    ComPtr<ID3D12Resource> m_pDepthStencilBuffer;
    ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;

    UINT m_nRtvDescriptorSize;
    UINT m_nDsvDescriptorSize;
    UINT m_nWidth;
    UINT m_nHeight;
};
