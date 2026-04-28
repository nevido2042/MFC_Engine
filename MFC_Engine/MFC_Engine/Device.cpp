#include "pch.h"
#include "Device.h"

CDevice::CDevice()
    : m_nRtvDescriptorSize(0)
    , m_nDsvDescriptorSize(0)
    , m_nFrameIndex(0)
    , m_hFenceEvent(nullptr)
    , m_nDebugFrameIndex(0)
{
    m_viewport = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    m_scissorRect = { 0, 0, 0, 0 };
    m_debugViewport = m_viewport;
    m_debugScissorRect = m_scissorRect;
    for (UINT i = 0; i < FrameCount; i++) m_nFenceValues[i] = 0;
}

CDevice::~CDevice()
{
    if (m_pDevice != nullptr)
    {
        WaitForPreviousFrame();
        if (m_hFenceEvent) CloseHandle(m_hFenceEvent);
    }
}

bool CDevice::Initialize(HWND hWnd, int width, int height)
{
    CreateDevice();
    CreateCommandQueue();
    CreateSwapChain(hWnd, width, height);
    CreateDescriptorHeaps();
    CreateRenderTargets();
    CreateDepthStencilBuffer(width, height);
    CreateGBuffers(width, height);
    
    // Command Allocator & List
    ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator)));
    ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList)));
    m_pCommandList->Close();

    // Fence
    ThrowIfFailed(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
    for (UINT i = 0; i < FrameCount; i++) m_nFenceValues[i] = 1;

    m_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_hFenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    m_viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    m_scissorRect = { 0, 0, width, height };

    return true;
}

void CDevice::Resize(int width, int height)
{
    if (m_pDevice == nullptr) return;
    if (width == 0 || height == 0) return;

    WaitForPreviousFrame();

    for (UINT n = 0; n < FrameCount; n++)
    {
        m_pRenderTargets[n].Reset();
    }

    DXGI_SWAP_CHAIN_DESC desc = {};
    m_pSwapChain->GetDesc(&desc);
    ThrowIfFailed(m_pSwapChain->ResizeBuffers(FrameCount, width, height, desc.BufferDesc.Format, desc.Flags));

    m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    CreateRenderTargets();
    CreateDepthStencilBuffer(width, height);
    CreateGBuffers(width, height);

    m_viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    m_scissorRect = { 0, 0, width, height };
}

void CDevice::PrepareRender()
{
    m_pCommandAllocator->Reset();
    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_nFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRtvHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDsvHandle();

    const float clearColor[] = { 0.2f, 0.2f, 0.3f, 1.0f };
    m_pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_pCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    m_pCommandList->RSSetViewports(1, &m_viewport);
    m_pCommandList->RSSetScissorRects(1, &m_scissorRect);
}

void CDevice::SubmitRender()
{
    m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pRenderTargets[m_nFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_pCommandList->Close());

    ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    ThrowIfFailed(m_pSwapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void CDevice::WaitForPreviousFrame()
{
    UINT64 fenceValue = m_nFenceValues[m_nFrameIndex]++;
    ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), fenceValue));

    if (m_pFence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(m_pFence->SetEventOnCompletion(fenceValue, m_hFenceEvent));
        WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
    
    m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void CDevice::WaitGPU()
{
    // 현재 프레임 인덱스와 무관하게 새로운 펜스 값을 발행하여 대기
    static UINT64 waitValue = 100000; // 충분히 큰 값부터 시작하거나 별도 관리
    waitValue++;
    
    ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), waitValue));
    
    if (m_pFence->GetCompletedValue() < waitValue)
    {
        ThrowIfFailed(m_pFence->SetEventOnCompletion(waitValue, m_hFenceEvent));
        WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE CDevice::GetRtvHandle() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += m_nFrameIndex * m_nRtvDescriptorSize;
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE CDevice::GetDsvHandle() const
{
    return m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE CDevice::GetGBufferRtvHandle(int index) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pGBufferRtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += index * m_nRtvDescriptorSize;
    return handle;
}

void CDevice::TransitionGBuffersToRenderTarget()
{
    D3D12_RESOURCE_BARRIER barriers[3];
    for (int i = 0; i < 3; ++i)
    {
        barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_pGBuffers[i].Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
    }
    m_pCommandList->ResourceBarrier(3, barriers);
}

void CDevice::TransitionGBuffersToPixelShaderResource()
{
    D3D12_RESOURCE_BARRIER barriers[3];
    for (int i = 0; i < 3; ++i)
    {
        barriers[i] = CD3DX12_RESOURCE_BARRIER::Transition(
            m_pGBuffers[i].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );
    }
    m_pCommandList->ResourceBarrier(3, barriers);
}

void CDevice::ClearAndSetGBuffers()
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3];
    for (int i = 0; i < 3; ++i)
    {
        rtvHandles[i] = GetGBufferRtvHandle(i);
        const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_pCommandList->ClearRenderTargetView(rtvHandles[i], clearColor, 0, nullptr);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDsvHandle();
    m_pCommandList->OMSetRenderTargets(3, rtvHandles, FALSE, &dsvHandle);
}

void CDevice::SetMainRenderTarget()
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRtvHandle();
    m_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr); // no depth for lighting pass
}

bool CDevice::InitializeDebugSwapChain(HWND hWnd, int width, int height)
{
    if (m_pDevice == nullptr || m_pCommandQueue == nullptr) return false;

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
    ThrowIfFailed(factory->CreateSwapChainForHwnd(m_pCommandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain));
    ThrowIfFailed(factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain.As(&m_pDebugSwapChain));
    m_nDebugFrameIndex = m_pDebugSwapChain->GetCurrentBackBufferIndex();

    // RTV Heap for Debug
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pDebugRtvHeap)));

    ResizeDebugSwapChain(width, height);

    return true;
}

void CDevice::ResizeDebugSwapChain(int width, int height)
{
    if (m_pDebugSwapChain == nullptr) return;
    if (width <= 0 || height <= 0) return;

    WaitForPreviousFrame();

    for (UINT i = 0; i < FrameCount; i++) m_pDebugRenderTargets[i].Reset();

    DXGI_SWAP_CHAIN_DESC desc = {};
    m_pDebugSwapChain->GetDesc(&desc);
    ThrowIfFailed(m_pDebugSwapChain->ResizeBuffers(FrameCount, width, height, desc.BufferDesc.Format, desc.Flags));
    m_nDebugFrameIndex = m_pDebugSwapChain->GetCurrentBackBufferIndex();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pDebugRtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < FrameCount; i++)
    {
        ThrowIfFailed(m_pDebugSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pDebugRenderTargets[i])));
        m_pDevice->CreateRenderTargetView(m_pDebugRenderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_nRtvDescriptorSize;
    }

    m_debugViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    m_debugScissorRect = { 0, 0, width, height };
}

bool CDevice::PrepareDebugRender()
{
    if (!m_pDebugSwapChain) return false;

    m_pCommandAllocator->Reset();
    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDebugRenderTargets[m_nDebugFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetDebugRtvHandle();
    const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    m_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    m_pCommandList->RSSetViewports(1, &m_debugViewport);
    m_pCommandList->RSSetScissorRects(1, &m_debugScissorRect);

    return true;
}

void CDevice::SubmitDebugRender()
{
    if (!m_pDebugSwapChain) return;

    m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDebugRenderTargets[m_nDebugFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_pCommandList->Close());

    ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    ThrowIfFailed(m_pDebugSwapChain->Present(1, 0));

    m_nDebugFrameIndex = m_pDebugSwapChain->GetCurrentBackBufferIndex();

    WaitForPreviousFrame();
}

D3D12_CPU_DESCRIPTOR_HANDLE CDevice::GetDebugRtvHandle() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pDebugRtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += m_nDebugFrameIndex * m_nRtvDescriptorSize;
    return handle;
}

void CDevice::CreateDevice()
{
    UINT dxgiFactoryFlags = 0;
    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice))))
    {
        ComPtr<IDXGIAdapter1> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice)));
    }
}

void CDevice::CreateCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ThrowIfFailed(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));
}

void CDevice::CreateSwapChain(HWND hWnd, int width, int height)
{
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
    ThrowIfFailed(factory->CreateSwapChainForHwnd(m_pCommandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain));
    ThrowIfFailed(factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain.As(&m_pSwapChain));
    m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void CDevice::CreateDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap)));
    m_nRtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap)));
    m_nDsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CDevice::CreateRenderTargets()
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT n = 0; n < FrameCount; n++)
    {
        ThrowIfFailed(m_pSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_pRenderTargets[n])));
        m_pDevice->CreateRenderTargetView(m_pRenderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_nRtvDescriptorSize;
    }
}

void CDevice::CreateDepthStencilBuffer(int width, int height)
{
    D3D12_RESOURCE_DESC depthResourceDesc = {};
    depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthResourceDesc.Width = width;
    depthResourceDesc.Height = height;
    depthResourceDesc.DepthOrArraySize = 1;
    depthResourceDesc.MipLevels = 1;
    depthResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthResourceDesc.SampleDesc.Count = 1;
    depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES depthHeapProps = {};
    depthHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    ThrowIfFailed(m_pDevice->CreateCommittedResource(
        &depthHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthResourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(&m_pDepthStencilBuffer)
    ));

    m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, m_pDsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void CDevice::CreateGBuffers(int width, int height)
{
    if (m_pDevice == nullptr) return;

    // RTV Heap for G-Buffers
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 3;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pGBufferRtvHeap)));

    // SRV Heap for G-Buffers
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 3;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_pGBufferSrvHeap)));

    DXGI_FORMAT formats[3] = {
        DXGI_FORMAT_R32G32B32A32_FLOAT, // Position
        DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
        DXGI_FORMAT_R8G8B8A8_UNORM      // Albedo
    };

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pGBufferRtvHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = m_pGBufferSrvHeap->GetCPUDescriptorHandleForHeapStart();
    UINT srvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    for (int i = 0; i < 3; ++i)
    {
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width = width;
        desc.Height = height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = formats[i];
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = formats[i];
        clearValue.Color[0] = 0.0f;
        clearValue.Color[1] = 0.0f;
        clearValue.Color[2] = 0.0f;
        clearValue.Color[3] = 0.0f;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

        ThrowIfFailed(m_pDevice->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            &clearValue,
            IID_PPV_ARGS(&m_pGBuffers[i])));

        // Create RTV
        m_pDevice->CreateRenderTargetView(m_pGBuffers[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_nRtvDescriptorSize;

        // Create SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = formats[i];
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        m_pDevice->CreateShaderResourceView(m_pGBuffers[i].Get(), &srvDesc, srvHandle);
        srvHandle.ptr += srvDescriptorSize;
    }
}

