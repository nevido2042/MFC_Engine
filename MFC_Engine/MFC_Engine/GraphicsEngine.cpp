#include "pch.h"
#include "d3dx12.h"
#include "GraphicsEngine.h"
#include <d3dcompiler.h>
#include "Scene.h"
#include "Transform.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"
#include "Mesh.h"

// 쉐이더 컴파일 라이브러리 링크
#pragma comment(lib, "d3dcompiler.lib")

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

    // --- 렌더링 파이프라인 구축 ---
    CreateRootSignature();
    CreatePipelineState();
    CreatePrimitiveMeshes();
    CreateConstantBuffer();
    CreateDepthStencilBuffer();

    // 초기 명령 리스트 생성 및 닫기
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
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
    // RTV Heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // DSV Heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
    m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

/**
 * @brief 스왑체인으로부터 실제 그릴 도화지(Render Target) 리소스를 가져옵니다.
 */
void CGraphicsEngine::CreateRenderTargets()
{
    // 수동으로 힙 핸들 계산 (d3dx12 보조 없이)
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT n = 0; n < FrameCount; n++)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_rtvDescriptorSize;
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
 * @brief 쉐이더 자원 바인딩을 위한 루트 시그니처를 생성합니다.
 */
void CGraphicsEngine::CreateRootSignature()
{
    // 상수 버퍼를 위한 루트 파라미터 정의
    D3D12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].Descriptor.RegisterSpace = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // 기본 루트 시그니처 구조체 직접 작성
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

/**
 * @brief 그래픽 파이프라인 상태 객체(PSO)를 생성합니다.
 */
void CGraphicsEngine::CreatePipelineState()
{
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    // 쉐이더 파일 컴파일
    ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

    // 입력 레이아웃 정의
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 파이프라인 상태 객체(PSO) 상세 설정 (Raw Struct 직접 채우기)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    
    // 기본 래스터라이저 설정
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.RasterizerState.MultisampleEnable = FALSE;
    psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    psoDesc.RasterizerState.ForcedSampleCount = 0;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // 기본 블렌드 설정
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

    // 깊이 스텐실 설정
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void CGraphicsEngine::CreatePrimitiveMeshes()
{
    m_meshes[L"Cube"] = CPrimitiveGenerator::CreateCubeMesh(m_device);
    m_meshes[L"Plane"] = CPrimitiveGenerator::CreatePlaneMesh(m_device);
    m_meshes[L"Quad"] = CPrimitiveGenerator::CreateQuadMesh(m_device);
    m_meshes[L"Sphere"] = CPrimitiveGenerator::CreateSphereMesh(m_device);
    m_meshes[L"Capsule"] = CPrimitiveGenerator::CreateCapsuleMesh(m_device);
}

std::shared_ptr<CMesh> CGraphicsEngine::GetPrimitiveMesh(const std::wstring& name)
{
    auto it = m_meshes.find(name);
    if (it != m_meshes.end())
    {
        return it->second;
    }
    return nullptr;
}

/**
 * @brief 쉐이더 상수를 담을 버퍼를 생성하고 메모리 매핑을 수행합니다.
 */
void CGraphicsEngine::CreateConstantBuffer()
{
    const UINT constantBufferSize = 256 * 1024; // 1024 objects * 256 bytes

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Alignment = 0;
    resDesc.Width = constantBufferSize;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ThrowIfFailed(m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer)));

    // 상수 버퍼는 CPU에서 매 프레임 쓰기 위해 Map을 유지합니다.
    D3D12_RANGE readRange = { 0, 0 };
    ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
    
    // 초기값 (단위 행렬) 설정
    SceneConstantBuffer cb;
    DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixIdentity());
    memcpy(m_pCbvDataBegin, &cb, sizeof(cb));
}

void CGraphicsEngine::CreateDepthStencilBuffer()
{
    D3D12_RESOURCE_DESC depthResourceDesc = {};
    depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthResourceDesc.Alignment = 0;
    depthResourceDesc.Width = m_width;
    depthResourceDesc.Height = m_height;
    depthResourceDesc.DepthOrArraySize = 1;
    depthResourceDesc.MipLevels = 1;
    depthResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthResourceDesc.SampleDesc.Count = 1;
    depthResourceDesc.SampleDesc.Quality = 0;
    depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES depthHeapProps = {};
    depthHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    ThrowIfFailed(m_device->CreateCommittedResource(
        &depthHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthResourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(&m_depthStencilBuffer)
    ));

    m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

/**
 * @brief 실시간 렌더링 루프를 수행합니다.
 */
void CGraphicsEngine::Render()
{
    Render(nullptr);
}

void CGraphicsEngine::Render(std::shared_ptr<CScene> pScene)
{
    if (!m_isInitialized) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    m_timeManager.Update();

    HRESULT hr = m_device->GetDeviceRemovedReason();
    if (FAILED(hr)) return;

    // 1. 명령 할당자 및 명령 목록 리셋
    ThrowIfFailed(m_commandAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

    // 2. 공통 상태 설정
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // 3. 렌더 타겟 준비 (Transition to RENDER_TARGET)
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_frameIndex * m_rtvDescriptorSize;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 4. 화면 지우기
    const float clearColor[] = { 0.2f, 0.2f, 0.3f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 5. 기하학적 형태 및 정점 버퍼 설정
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 6. 씬 오브젝트 렌더링
    if (pScene)
    {
        const auto& gameObjects = pScene->GetGameObjects();
        int objIndex = 0;

        for (auto& pObj : gameObjects)
        {
            RenderGameObject(pObj, objIndex);
        }
    }
    else
    {
        // 씬이 없을 경우 기본 회전 큐브 하나만 그림 (디버그용)
        float totalTime = m_timeManager.GetTotalTime();
        DirectX::XMMATRIX matWorld = DirectX::XMMatrixRotationRollPitchYaw(totalTime * 0.5f, totalTime, 0.0f);
        DirectX::XMMATRIX matView = m_camera.GetViewMatrix();
        float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
        DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 100.0f);
        DirectX::XMMATRIX matWVP = matWorld * matView * matProj;

        SceneConstantBuffer cb;
        DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixTranspose(matWVP));
        memcpy(m_pCbvDataBegin, &cb, sizeof(cb));

        m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());
        
        auto it = m_meshes.find(L"Cube");
        if (it != m_meshes.end())
        {
            it->second->Render(m_commandList);
        }
    }

    // 7. 프레젠테이션으로 전환
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());

    // 8. 명령 실행
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // 9. 화면 출력
    ThrowIfFailed(m_swapChain->Present(1, 0));
    WaitForPreviousFrame();
}

void CGraphicsEngine::Resize(int width, int height)
{
    if (!m_isInitialized) return;
    if (width == 0 || height == 0) return;
    if (m_width == width && m_height == height) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    WaitForPreviousFrame();

    for (UINT n = 0; n < FrameCount; n++)
    {
        m_renderTargets[n].Reset();
    }

    DXGI_SWAP_CHAIN_DESC desc = {};
    m_swapChain->GetDesc(&desc);
    ThrowIfFailed(m_swapChain->ResizeBuffers(FrameCount, width, height, desc.BufferDesc.Format, desc.Flags));

    m_width = width;
    m_height = height;
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    CreateRenderTargets();

    m_viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    m_scissorRect = { 0, 0, width, height };

    CreateDepthStencilBuffer();
}



void CGraphicsEngine::RenderGameObject(std::shared_ptr<CGameObject> pObj, int& objIndex)
{
    if (objIndex >= 1024) return;

    auto pRenderer = pObj->GetComponent<CMeshRenderer>();
    if (pRenderer && pRenderer->m_isEnabled)
    {
        auto pFilter = pObj->GetComponent<CMeshFilter>();
        if (pFilter)
        {
            auto pTransform = pObj->GetTransform();
            if (pTransform)
            {
                // 행렬 계산
                DirectX::XMMATRIX matWorld = pTransform->GetWorldMatrix();
                DirectX::XMMATRIX matView = m_camera.GetViewMatrix();
                float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
                DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 100.0f);

                DirectX::XMMATRIX matWVP = matWorld * matView * matProj;

                // 상수 버퍼 업데이트
                SceneConstantBuffer cb;
                DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixTranspose(matWVP));
                memcpy(m_pCbvDataBegin + (objIndex * 256), &cb, sizeof(cb));

                // 해당 오브젝트의 CBV 연결 및 그리기
                m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + (objIndex * 256));

                auto it = m_meshes.find(pFilter->m_meshName);
                if (it != m_meshes.end())
                {
                    it->second->Render(m_commandList);
                }

                objIndex++;
            }
        }
    }

    // 자식들도 렌더링
    for (auto& pChild : pObj->GetChildren())
    {
        RenderGameObject(pChild, objIndex);
    }
}

void CGraphicsEngine::WaitForPreviousFrame()
{
    UINT64 fenceValue = m_fenceValues[m_frameIndex]++;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fenceValue));

    if (m_fence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
    
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void CGraphicsEngine::MoveCamera(float forward, float right, float up, float deltaTime)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_camera.Move(forward, right, up, deltaTime);
}

void CGraphicsEngine::RotateCamera(float pitch, float yaw)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_camera.Rotate(pitch, yaw);
}
