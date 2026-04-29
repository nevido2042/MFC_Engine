#include "pch.h"
#include "GraphicsEngine.h"
#include "Scene.h"
#include "Transform.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "Light.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "DebugPass.h"
#include "PickingSystem.h"
#include "SceneManager.h"
#include "ImGuiManager.h"

// 쉐이더 컴파일 라이브러리 링크

CGraphicsEngine::CGraphicsEngine()
    : m_bIsInitialized(false)
    , m_nWidth(0)
    , m_nHeight(0)
{
}

CGraphicsEngine::~CGraphicsEngine()
{
    m_pDevice->WaitForGPU();
}

/**
 * @brief 엔진의 모든 DX12 초기 설정을 수행합니다.
 */
bool CGraphicsEngine::Initialize(HWND hWnd, int width, int height)
{
    m_nWidth = (width > 0) ? width : 1;
    m_nHeight = (height > 0) ? height : 1;

    m_timeManager.Initialize();

    m_pDevice = std::make_unique<CDevice>();
    m_pDevice->Initialize();

    m_pMainSwapChain = std::make_unique<CSwapChain>();
    m_pMainSwapChain->Initialize(m_pDevice.get(), hWnd, m_nWidth, m_nHeight);

    m_pGBuffer = std::make_unique<CGBuffer>();
    m_pGBuffer->Initialize(m_pDevice.get(), m_nWidth, m_nHeight);

    // 렌더 패스 초기화
    m_pGeometryPass = std::make_unique<CGeometryPass>();
    m_pGeometryPass->Initialize(m_pDevice->GetDevice());

    m_pLightingPass = std::make_unique<CLightingPass>();
    m_pLightingPass->Initialize(m_pDevice->GetDevice());

    m_pDebugPass = std::make_unique<CDebugPass>();
    m_pDebugPass->Initialize(m_pDevice->GetDevice());

    CPickingSystem::GetInstance().Initialize(m_pDevice->GetDevice(), m_pGeometryPass->GetRootSignature(), m_nWidth, m_nHeight);
    CPrimitiveGenerator::GetInstance().Initialize(m_pDevice->GetDevice());
    CreateConstantBuffer();

    // --- ImGui Manager 초기화 ---
    m_pImGuiManager = std::make_unique<CImGuiManager>();
    if (!m_pImGuiManager->Initialize(hWnd, m_pDevice->GetDevice(), m_pDevice->GetCommandQueue(), 2))
        return false;

    m_bIsInitialized = true;
    return true;
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

    // Transition Main SwapChain to Render Target (was done in PrepareRender before)
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

    // Pass 1: Geometry Pass
    m_pGeometryPass->Execute(context);

    // Pass 2: Lighting Pass
    m_pLightingPass->Execute(context);

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

    // Pass 1: Geometry Pass
    m_pGeometryPass->Execute(context);

    // Pass 2: Lighting Pass
    m_pLightingPass->Execute(context);

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
    context.nWidth = m_nWidth;
    context.nHeight = m_nHeight;

    context.resources.pMainSwapChain = m_pDebugSwapChain.get();
    context.resources.pGBuffer = m_pGBuffer.get();

    m_pDebugPass->Execute(context);

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
    m_pMainSwapChain->Resize(m_pDevice.get(), width, height);
    m_pGBuffer->Resize(m_pDevice.get(), width, height);
    
    CPickingSystem::GetInstance().Resize(m_pDevice->GetDevice(), width, height);
}

bool CGraphicsEngine::InitializeDebugSwapChain(HWND hWnd, int width, int height)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_pDevice) return false;
    
    if (!m_pDebugSwapChain)
        m_pDebugSwapChain = std::make_unique<CSwapChain>();
        
    m_pDebugSwapChain->Initialize(m_pDevice.get(), hWnd, width, height);
    return true;
}

void CGraphicsEngine::ResizeDebugSwapChain(int width, int height)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pDebugSwapChain)
    {
        m_pDevice->WaitForGPU();
        m_pDebugSwapChain->Resize(m_pDevice.get(), width, height);
    }
}
