#pragma once

#include "RenderPass.h"
// 인라인 전방 선언 사용으로 외부 클래스 의존성 제거
#include "TimeManager.h"
#include "PrimitiveGenerator.h"
#include "ConstantBuffer.h"
#include "Device.h"
#include "SwapChain.h"
#include "GBuffer.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "DebugPass.h"
#include "GizmoPass.h"
// ImGui 로직은 CImGuiManager에서 관리
class CImGuiManager;

/**
 * @class CGraphicsEngine
 * @brief DirectX 12 기반의 그래픽 렌더링 핵심 엔진 클래스입니다.
 */
class CGraphicsEngine
{
public:
    CGraphicsEngine();
    ~CGraphicsEngine();

    // --- 핵심 인터페이스 ---
    bool Initialize(HWND hWnd, int width, int height);
    void Render(std::shared_ptr<class CScene> pScene, std::shared_ptr<class CGameObject> pSelectedObj, class CGizmo* pGizmo);
    void RenderDebugGBuffers();
    bool InitializeDebugSwapChain(HWND hWnd, int width, int height);
    void ResizeDebugSwapChain(int width, int height);
    void Resize(int width, int height);
    float GetFPS() const { return m_timeManager.GetFPS(); }
    ID3D12Device* GetDevice() { return m_pDevice ? m_pDevice->GetDevice() : nullptr; }
    
    void SetDebugViewActive(bool bActive) { m_bDebugViewActive = bActive; }
    bool IsDebugViewActive() const { return m_bDebugViewActive; }
    
    // --- 리소스 접근자 (CPickingSystem 등에서 사용) ---
    ID3D12CommandQueue* GetCommandQueue() { return m_pDevice->GetCommandQueue(); }
    ID3D12CommandAllocator* GetCommandAllocator() { return m_pDevice->GetCommandAllocator(); }
    ID3D12GraphicsCommandList* GetCommandList() { return m_pDevice->GetCommandList(); }
    ID3D12RootSignature* GetRootSignature() { return m_pGeometryPass ? m_pGeometryPass->GetRootSignature() : nullptr; }
    ID3D12Resource* GetConstantBufferResource() { return m_pConstantBuffer->GetResource(); }
    UINT8* GetConstantBufferPtr() { return m_pConstantBuffer->GetMappedData(); }
    int GetWidth() const { return m_nWidth; }
    int GetHeight() const { return m_nHeight; }
    ID3D12Resource* GetGBufferResource(int index) { return m_pGBuffer ? m_pGBuffer->GetResource(index) : nullptr; }

    void WaitGPU() { if (m_pDevice) m_pDevice->WaitForGPU(); }
    std::mutex& GetMutex() { return m_mutex; }

    void PrepareCommandList() { if (m_pDevice) m_pDevice->PrepareCommandList(); }
    void SubmitCommandList() { if (m_pDevice) m_pDevice->SubmitCommandList(); }

private:
    void CreateConstantBuffer();    // 상수 버퍼 생성


private:
    // --- DX12 핵심 장치 ---
    std::unique_ptr<CDevice> m_pDevice;
    std::unique_ptr<CSwapChain> m_pMainSwapChain;
    std::unique_ptr<CSwapChain> m_pDebugSwapChain;
    std::unique_ptr<CGBuffer> m_pGBuffer;

    // --- 렌더 패스 ---
    std::unique_ptr<class CGeometryPass> m_pGeometryPass;
    std::unique_ptr<class CLightingPass> m_pLightingPass;
    std::unique_ptr<class CGizmoPass> m_pGizmoPass;
    std::unique_ptr<class CDebugPass> m_pDebugPass;
    
    std::unique_ptr<CConstantBuffer> m_pConstantBuffer; // 상수 버퍼 매니저

    std::unique_ptr<class CImGuiManager> m_pImGuiManager;


    // --- 동기화 및 상태 변수 ---
    bool m_bIsInitialized;
    bool m_bDebugViewActive = false;
    int m_nWidth;
    int m_nHeight;

    // --- 매니저 객체 분리 ---
    CTimeManager m_timeManager;

    // --- 스레드 동기화 ---
    std::mutex m_mutex;
};
