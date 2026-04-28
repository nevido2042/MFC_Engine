#include "pch.h"
#include "SwapChain.h"
#include "Device.h"

CSwapChain::CSwapChain()
    : m_nRtvDescriptorSize(0)
    , m_nFrameIndex(0)
{
}

CSwapChain::~CSwapChain()
{
}

void CSwapChain::Initialize(CDevice* pDevice, HWND hWnd, int width, int height)
{
    if (!pDevice || !pDevice->GetDevice() || !pDevice->GetCommandQueue()) return;

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(pDevice->GetCommandQueue(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain));
    ThrowIfFailed(factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain.As(&m_pSwapChain));
    m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    CreateRtvHeap(pDevice);
    Resize(pDevice, width, height);
}

void CSwapChain::CreateRtvHeap(CDevice* pDevice)
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(pDevice->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap)));
    m_nRtvDescriptorSize = pDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void CSwapChain::Resize(CDevice* pDevice, int width, int height)
{
    if (m_pSwapChain == nullptr) return;
    if (width <= 0 || height <= 0) return;

    pDevice->WaitForGPU();

    ClearRenderTargets();

    DXGI_SWAP_CHAIN_DESC desc = {};
    m_pSwapChain->GetDesc(&desc);
    ThrowIfFailed(m_pSwapChain->ResizeBuffers(FrameCount, width, height, desc.BufferDesc.Format, desc.Flags));
    m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    CreateRenderTargets(pDevice);

    m_viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
    m_scissorRect = { 0, 0, width, height };
}

void CSwapChain::ClearRenderTargets()
{
    for (UINT i = 0; i < FrameCount; i++)
    {
        m_pRenderTargets[i].Reset();
    }
}

void CSwapChain::CreateRenderTargets(CDevice* pDevice)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < FrameCount; i++)
    {
        ThrowIfFailed(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pRenderTargets[i])));
        pDevice->GetDevice()->CreateRenderTargetView(m_pRenderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_nRtvDescriptorSize;
    }
}

void CSwapChain::Present()
{
    if (m_pSwapChain)
    {
        ThrowIfFailed(m_pSwapChain->Present(1, 0));
        m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE CSwapChain::GetRtvHandle() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += m_nFrameIndex * m_nRtvDescriptorSize;
    return handle;
}

ID3D12Resource* CSwapChain::GetRenderTarget() const
{
    return m_pRenderTargets[m_nFrameIndex].Get();
}
