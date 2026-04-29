#include "pch.h"
#include "GraphicsEngine.h"
#include "Scene.h"
#include "Transform.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "Light.h"
#include "RenderPass.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "DebugPass.h"
#include "PickingSystem.h"
#include "PrimitiveGenerator.h"
#include "SceneManager.h"
#include "ImGuiManager.h"
#include "Device.h"
#include "SwapChain.h"
#include "GBuffer.h"
#include "ConstantBuffer.h"

// 쉐이더 컴파일 라이브러리 링크

CGraphicsEngine::CGraphicsEngine()
    : m_bIsInitialized(false)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_pMainGeometryPass(nullptr)
{
}

CGraphicsEngine::~CGraphicsEngine()
{
}

/**
 * @brief 엔진의 모든 DX12 초기 설정을 수행합니다.
 */
bool CGraphicsEngine::Initialize(CDevice* pDevice, int width, int height)
{
    if (!pDevice) return false;
    m_pDevice = pDevice;

    m_nWidth = (width > 0) ? width : 1;
    m_nHeight = (height > 0) ? height : 1;

    m_timeManager.Initialize();

    // 상수 버퍼 생성 (엔진 내부 리소스)
    CreateConstantBuffer();

    m_bIsInitialized = true;
    return true;
}

void CGraphicsEngine::SetSwapChain(std::unique_ptr<CSwapChain>&& pSwapChain)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pMainSwapChain = std::move(pSwapChain);
}

void CGraphicsEngine::SetGBuffer(std::unique_ptr<CGBuffer>&& pGBuffer)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pGBuffer = std::move(pGBuffer);
}

void CGraphicsEngine::SetImGuiManager(std::unique_ptr<CImGuiManager>&& pImGui)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pImGuiManager = std::move(pImGui);
}

void CGraphicsEngine::SetPickingSystem(std::unique_ptr<CPickingSystem>&& pPicking)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pPickingSystem = std::move(pPicking);
}

void CGraphicsEngine::SetPrimitiveGenerator(std::unique_ptr<CPrimitiveGenerator>&& pGenerator)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pPrimitiveGenerator = std::move(pGenerator);
}

bool CGraphicsEngine::InitializeDebugSwapChain(HWND hWnd, int width, int height)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_pDevice) return false;

    m_pDebugSwapChain = std::make_unique<CSwapChain>();
    m_pDebugSwapChain->Initialize(m_pDevice, hWnd, width, height);

    return true;
}

void CGraphicsEngine::ResizeDebugSwapChain(int width, int height)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pDebugSwapChain && m_pDevice)
    {
        m_pDebugSwapChain->Resize(m_pDevice, width, height);
    }
}

/**
 * @brief 쉐이더 상수를 담을 버퍼를 생성하고 메모리 매핑을 수행합니다.
 */
void CGraphicsEngine::CreateConstantBuffer()
{
    m_pConstantBuffer = std::make_unique<CConstantBuffer>();
    m_pConstantBuffer->Initialize(m_pDevice->GetDevice(), sizeof(SceneConstantBuffer), 1024);
}


void CGraphicsEngine::Render(std::shared_ptr<CScene> pScene, std::shared_ptr<CGameObject> pSelectedObj)
{
    if (!m_bIsInitialized) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_timeManager.Update();

    m_pDevice->PrepareCommandList(); // Reset command list

    // Get current render target resource from main swapchain
    auto rtv = m_pMainSwapChain->GetRenderTarget();
    auto commandList = m_pDevice->GetCommandList();

    // Transition Main SwapChain to Render Target
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    RenderContext context = {};
    context.pCommandList = commandList;
    context.pCB = m_pConstantBuffer.get();
    context.nWidth = m_nWidth;
    context.nHeight = m_nHeight;

    context.scene.pScene = pScene.get();
    context.scene.pSelectedObj = pSelectedObj.get();

    context.resources.pMainSwapChain = m_pMainSwapChain.get();
    context.resources.pGBuffer = m_pGBuffer.get();
    context.resources.pPickingSystem = m_pPickingSystem.get();
    context.resources.pPrimitiveGenerator = m_pPrimitiveGenerator.get();

    // --- 메인 렌더 패스 실행 ---
    for (const auto& pPass : m_mainRenderPasses)
    {
        pPass->Execute(context);
    }

    // --- ImGui / ImGuizmo Render ---
    m_pImGuiManager->NewFrame();

    // 뷰/투영 행렬 계산
    DirectX::XMMATRIX viewMat, projMat;
    {
        std::lock_guard<std::mutex> camLock(CSceneManager::GetInstance().GetCameraMutex());
        viewMat = CSceneManager::GetInstance().GetEditorCamera().GetViewMatrix();
    }
    float aspectRatio = static_cast<float>(m_nWidth) / static_cast<float>(m_nHeight);
    projMat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 1000.0f);

    // 기즈모 및 그리드 업데이트
    m_pImGuiManager->UpdateGizmo(pSelectedObj.get(), viewMat, projMat, m_nWidth, m_nHeight);

    // Pass 3: ImGui 최종 렌더링
    m_pImGuiManager->Render(commandList, m_pMainSwapChain->GetRtvHandle());

    // Transition Main SwapChain back to Present
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    m_pDevice->SubmitCommandList();
    m_pMainSwapChain->Present();
    m_pDevice->WaitForGPU();
}

void CGraphicsEngine::RenderDebugGBuffers()
{
    if (!m_bIsInitialized) return;
    if (!m_pDebugSwapChain || !m_pDebugSwapChain->GetRenderTarget()) return; // Debug 뷰가 초기화 안됐으면 그리지 않음

    std::lock_guard<std::mutex> lock(m_mutex);

    m_pDevice->PrepareCommandList();
    
    auto commandList = m_pDevice->GetCommandList();
    auto rtv = m_pDebugSwapChain->GetRenderTarget();

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Viewport & Scissor 설정
    commandList->RSSetViewports(1, &m_pDebugSwapChain->GetViewport());
    commandList->RSSetScissorRects(1, &m_pDebugSwapChain->GetScissorRect());

    // Render Target 설정 및 Clear
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pDebugSwapChain->GetRtvHandle();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    
    const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    RenderContext context = {};
    context.pCommandList = commandList;
    context.pCB = m_pConstantBuffer.get();
    context.nWidth = m_pDebugSwapChain->GetWidth();
    context.nHeight = m_pDebugSwapChain->GetHeight();

    context.resources.pMainSwapChain = m_pDebugSwapChain.get();
    context.resources.pGBuffer = m_pGBuffer.get();
    context.resources.pPickingSystem = m_pPickingSystem.get();
    context.resources.pPrimitiveGenerator = m_pPrimitiveGenerator.get();

    if (m_pDebugPass)
    {
        m_pDebugPass->Execute(context);
    }

    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    m_pDevice->SubmitCommandList();
    m_pDebugSwapChain->Present();
    m_pDevice->WaitForGPU();
}

void CGraphicsEngine::Resize(int width, int height)
{
    if (!m_bIsInitialized) return;
    if (width == 0 || height == 0) return;
    if (m_nWidth == width && m_nHeight == height) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    m_nWidth = width;
    m_nHeight = height;

    m_pDevice->WaitForGPU();
    m_pMainSwapChain->Resize(m_pDevice, width, height);
    m_pGBuffer->Resize(m_pDevice, width, height);
    
    if (m_pPickingSystem)
    {
        m_pPickingSystem->Resize(m_pDevice->GetDevice(), width, height);
    }
}

ID3D12RootSignature* CGraphicsEngine::GetRootSignature()
{
    return m_pMainGeometryPass ? m_pMainGeometryPass->GetRootSignature() : nullptr;
}

void CGraphicsEngine::RegisterRenderPass(std::unique_ptr<CRenderPass> pPass, bool bIsMainPass)
{
    if (bIsMainPass)
    {
        // CGeometryPass인 경우 포인터 별도 보관 (RootSignature 접근용)
        CGeometryPass* pGeo = dynamic_cast<CGeometryPass*>(pPass.get());
        if (pGeo) m_pMainGeometryPass = pGeo;

        m_mainRenderPasses.push_back(std::move(pPass));
    }
}

void CGraphicsEngine::WaitGPU()
{
    if (m_pDevice)
        m_pDevice->WaitForGPU();
}

float CGraphicsEngine::GetFPS() const
{
    return m_timeManager.GetFPS();
}

ID3D12Device* CGraphicsEngine::GetDevice()
{
    return m_pDevice ? m_pDevice->GetDevice() : nullptr;
}

ID3D12CommandQueue* CGraphicsEngine::GetCommandQueue()
{
    return m_pDevice ? m_pDevice->GetCommandQueue() : nullptr;
}

ID3D12CommandAllocator* CGraphicsEngine::GetCommandAllocator()
{
    return m_pDevice ? m_pDevice->GetCommandAllocator() : nullptr;
}

ID3D12GraphicsCommandList* CGraphicsEngine::GetCommandList()
{
    return m_pDevice ? m_pDevice->GetCommandList() : nullptr;
}

ID3D12Resource* CGraphicsEngine::GetConstantBufferResource()
{
    return m_pConstantBuffer ? m_pConstantBuffer->GetResource() : nullptr;
}

UINT8* CGraphicsEngine::GetConstantBufferPtr()
{
    return m_pConstantBuffer ? m_pConstantBuffer->GetMappedData() : nullptr;
}

void CGraphicsEngine::PrepareCommandList()
{
    if (m_pDevice)
        m_pDevice->PrepareCommandList();
}

void CGraphicsEngine::SubmitCommandList()
{
    if (m_pDevice)
        m_pDevice->SubmitCommandList();
}

ID3D12Resource* CGraphicsEngine::GetGBufferResource(int nIndex)
{
    if (!m_pGBuffer) return nullptr;
    return m_pGBuffer->GetResource(nIndex);
}
