#include "pch.h"
#include "GraphicsEngine.h"

CGraphicsEngine::CGraphicsEngine()
    : m_frameIndex(0)
    , m_rtvDescriptorSize(0)
    , m_isInitialized(false)
    , m_width(0)
    , m_height(0)
{
    // 기본 뷰포트 및 가위 사각형 초기화
    m_viewport = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    m_scissorRect = { 0, 0, 0, 0 };
    for (UINT i = 0; i < FrameCount; i++) m_fenceValues[i] = 0;
}

CGraphicsEngine::~CGraphicsEngine()
{
    if (m_isInitialized)
    {
        // 종료 전 GPU 작업이 끝날 때까지 대기하여 리소스 해제 시 충돌 방지
        WaitForPreviousFrame();
        if (m_fenceEvent) CloseHandle(m_fenceEvent);
    }
}

/**
 * @brief 엔진의 모든 DX12 초기 설정을 수행합니다.
 */
bool CGraphicsEngine::Initialize(HWND hWnd, int width, int height)
{
    m_width = (width > 0) ? width : 1;
    m_height = (height > 0) ? height : 1;

    // 뷰포트 설정 (좌표계 정의)
    m_viewport = { 0.0f, 0.0f, (float)m_width, (float)m_height, 0.0f, 1.0f };
    m_scissorRect = { 0, 0, m_width, m_height };

    // 타임 매니저 초기화
    m_timeManager.Initialize();

    // DX12 파이프라인 구성 요소 생성 순차 실행
    CreateDevice();
    CreateCommandQueue();
    CreateSwapChain(hWnd, m_width, m_height);
    CreateDescriptorHeaps();
    CreateRenderTargets();
    CreateCommandAllocator();

    // 초기 명령 리스트 생성 및 닫기
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    m_commandList->Close();

    // 동기화를 위한 펜스 객체 생성
    ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    for (UINT i = 0; i < FrameCount; i++) m_fenceValues[i] = 1;

    // GPU 대기를 위한 이벤트 핸들 생성
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    m_isInitialized = true;
    return true;
}

/**
 * @brief 하드웨어 가속을 위한 DX12 장치를 생성합니다.
 */
void CGraphicsEngine::CreateDevice()
{
    UINT dxgiFactoryFlags = 0;
    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    // 하드웨어 어댑터(그래픽카드) 시도, 실패 시 WARP(소프트웨어 렌더러) 사용
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))))
    {
        ComPtr<IDXGIAdapter1> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));
    }
}

/**
 * @brief 명령어를 GPU로 전달할 큐를 생성합니다.
 */
void CGraphicsEngine::CreateCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 그래픽 명령 전용
    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
}

/**
 * @brief 윈도우 핸들과 연결된 스왑체인을 생성하여 더블 버퍼링을 지원합니다.
 */
void CGraphicsEngine::CreateSwapChain(HWND hWnd, int width, int height)
{
    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 현대적인 플립 방식
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(m_commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain));
    ThrowIfFailed(factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

/**
 * @brief 렌더 타겟 서술자(Descriptor)를 담을 메모리 힙을 생성합니다.
 */
void CGraphicsEngine::CreateDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

/**
 * @brief 스왑체인으로부터 실제 그릴 도화지(Render Target) 리소스를 가져옵니다.
 */
void CGraphicsEngine::CreateRenderTargets()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT n = 0; n < FrameCount; n++)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }
}

/**
 * @brief 명령어 기록에 필요한 임시 메모리(Allocator)를 생성합니다.
 */
void CGraphicsEngine::CreateCommandAllocator()
{
    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

/**
 * @brief 실시간 렌더링 루프를 수행합니다.
 */
void CGraphicsEngine::Render()
{
    if (!m_isInitialized) return;

    // --- 시간 업데이트 (분리된 매니저 사용) ---
    m_timeManager.Update();

    // 장치 소실 여부 체크
    HRESULT hr = m_device->GetDeviceRemovedReason();
    if (FAILED(hr))
    {
        CString str;
        str.Format(_T("Device Removed Reason: 0x%08X"), hr);
        AfxMessageBox(str);
        return;
    }

    // 명령 기록 시작
    ThrowIfFailed(m_commandAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

    // 리소스 상태 변경: Present -> RenderTarget
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // 배경색으로 화면 지우기 (Clear)
    const float clearColor[] = { 0.2f, 0.2f, 0.3f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // 리소스 상태 변경: RenderTarget -> Present
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());

    // GPU에 명령 제출
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // 화면 출력
    ThrowIfFailed(m_swapChain->Present(1, 0));

    // CPU-GPU 동기화 (프레임 안정성)
    WaitForPreviousFrame();
}

/**
 * @brief 윈도우 리사이즈 시 그래픽 버퍼를 다시 구성합니다.
 */
void CGraphicsEngine::Resize(int width, int height)
{
    if (!m_isInitialized) return;
    if (width == 0 || height == 0) return;
    if (m_width == width && m_height == height) return;

    // 실행 중인 GPU 작업 대기
    WaitForPreviousFrame();

    // 기존 렌더 타겟 리소스 해제
    for (UINT n = 0; n < FrameCount; n++)
    {
        m_renderTargets[n].Reset();
    }

    // 버퍼 크기 재조정
    DXGI_SWAP_CHAIN_DESC desc = {};
    m_swapChain->GetDesc(&desc);
    ThrowIfFailed(m_swapChain->ResizeBuffers(FrameCount, width, height, desc.BufferDesc.Format, desc.Flags));

    m_width = width;
    m_height = height;
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // 새 리소스로 뷰 생성
    CreateRenderTargets();

    // 뷰포트 정보 업데이트
    m_viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    m_scissorRect = { 0, 0, width, height };
}

/**
 * @brief GPU가 현재 프레임의 명령을 모두 처리할 때까지 CPU를 대기시킵니다.
 */
void CGraphicsEngine::WaitForPreviousFrame()
{
    UINT64 fenceValue = m_fenceValues[m_frameIndex]++;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceValue));

    // GPU가 아직 펜스 값에 도달하지 못했다면 이벤트 대기
    if (m_fence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
    
    // 다음 사용할 백 버퍼 인덱스 갱신
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}
