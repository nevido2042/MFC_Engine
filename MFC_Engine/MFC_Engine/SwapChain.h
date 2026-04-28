#pragma once
#include "pch.h"

class CDevice;

class CSwapChain
{
public:
    CSwapChain();
    ~CSwapChain();

    void Initialize(CDevice* pDevice, HWND hWnd, int width, int height);
    void Resize(CDevice* pDevice, int width, int height);
    void Present();

    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle() const;
    ID3D12Resource* GetRenderTarget() const;

    const D3D12_VIEWPORT& GetViewport() const { return m_viewport; }
    const D3D12_RECT& GetScissorRect() const { return m_scissorRect; }
    UINT GetCurrentBackBufferIndex() const { return m_nFrameIndex; }

private:
    void CreateRtvHeap(CDevice* pDevice);
    void CreateRenderTargets(CDevice* pDevice);
    void ClearRenderTargets();

private:
    static const UINT FrameCount = 2;

    ComPtr<IDXGISwapChain3> m_pSwapChain;
    ComPtr<ID3D12Resource> m_pRenderTargets[FrameCount];
    ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
    UINT m_nRtvDescriptorSize;
    UINT m_nFrameIndex;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;
};
