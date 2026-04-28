#include "pch.h"
#include "GBuffer.h"
#include "Device.h"

CGBuffer::CGBuffer()
    : m_nRtvDescriptorSize(0)
    , m_nDsvDescriptorSize(0)
    , m_nWidth(0)
    , m_nHeight(0)
{
}

CGBuffer::~CGBuffer()
{
}

void CGBuffer::Initialize(CDevice* pDevice, int width, int height)
{
    if (!pDevice || !pDevice->GetDevice()) return;

    CreateHeaps(pDevice);
    Resize(pDevice, width, height);
}

void CGBuffer::CreateHeaps(CDevice* pDevice)
{
    auto device = pDevice->GetDevice();

    // RTV Heap for G-Buffers
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = BufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap)));
    m_nRtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // SRV Heap for G-Buffers
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = BufferCount;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pSrvHeap)));

    // DSV Heap for Depth Stencil
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap)));
    m_nDsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGBuffer::Resize(CDevice* pDevice, int width, int height)
{
    if (width <= 0 || height <= 0) return;

    pDevice->WaitForGPU();

    m_nWidth = width;
    m_nHeight = height;

    ClearResources();
    CreateResources(pDevice, width, height);
}

void CGBuffer::ClearResources()
{
    for (UINT i = 0; i < BufferCount; ++i)
    {
        m_pGBuffers[i].Reset();
    }
    m_pDepthStencilBuffer.Reset();
}

void CGBuffer::CreateResources(CDevice* pDevice, int width, int height)
{
    auto device = pDevice->GetDevice();

    // 1. Create G-Buffer Resources
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    DXGI_FORMAT formats[BufferCount] = {
        DXGI_FORMAT_R32G32B32A32_FLOAT, // Position
        DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
        DXGI_FORMAT_R8G8B8A8_UNORM      // Albedo
    };

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 1.0f;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_pSrvHeap->GetCPUDescriptorHandleForHeapStart();
    UINT srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    for (UINT i = 0; i < BufferCount; ++i)
    {
        desc.Format = formats[i];
        clearValue.Format = formats[i];

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            &clearValue,
            IID_PPV_ARGS(&m_pGBuffers[i])
        ));

        // Create RTV
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = formats[i];
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        device->CreateRenderTargetView(m_pGBuffers[i].Get(), &rtvDesc, rtvHandle);
        rtvHandle.ptr += m_nRtvDescriptorSize;

        // Create SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = formats[i];
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;
        device->CreateShaderResourceView(m_pGBuffers[i].Get(), &srvDesc, srvHandle);
        srvHandle.ptr += srvDescriptorSize;
    }

    // 2. Create Depth Stencil Resource
    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthClearValue = {};
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthClearValue.DepthStencil.Depth = 1.0f;
    depthClearValue.DepthStencil.Stencil = 0;

    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(&m_pDepthStencilBuffer)
    ));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    device->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &dsvDesc, m_pDsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void CGBuffer::TransitionToRenderTarget(ID3D12GraphicsCommandList* pCmdList)
{
    D3D12_RESOURCE_BARRIER barriers[BufferCount];
    for (UINT i = 0; i < BufferCount; ++i)
    {
        barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_pGBuffers[i].Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
    }
    pCmdList->ResourceBarrier(BufferCount, barriers);
}

void CGBuffer::TransitionToShaderResource(ID3D12GraphicsCommandList* pCmdList)
{
    D3D12_RESOURCE_BARRIER barriers[BufferCount];
    for (UINT i = 0; i < BufferCount; ++i)
    {
        barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_pGBuffers[i].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );
    }
    pCmdList->ResourceBarrier(BufferCount, barriers);
}

void CGBuffer::ClearAndSet(ID3D12GraphicsCommandList* pCmdList)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[BufferCount];
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    for (UINT i = 0; i < BufferCount; ++i)
    {
        rtvHandles[i] = rtvHandle;
        pCmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        rtvHandle.ptr += m_nRtvDescriptorSize;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDsvHandle();
    pCmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    pCmdList->OMSetRenderTargets(BufferCount, rtvHandles, FALSE, &dsvHandle);
}

D3D12_CPU_DESCRIPTOR_HANDLE CGBuffer::GetDsvHandle() const
{
    return m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();
}
