#pragma once

// 인라인 전방 선언 사용으로 외부 클래스 의존성 제거
#include "TimeManager.h"
#include "PrimitiveGenerator.h"
#include "ConstantBuffer.h"
#include "Device.h"
#include "SwapChain.h"
#include "GBuffer.h"

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
    ID3D12RootSignature* GetRootSignature() { return m_rootSignatureDeferred.Get(); }
    ID3D12Resource* GetConstantBufferResource() { return m_pConstantBuffer->GetResource(); }
    UINT8* GetConstantBufferPtr() { return m_pConstantBuffer->GetMappedData(); }
    int GetWidth() const { return m_nWidth; }
    int GetHeight() const { return m_nHeight; }

    void WaitGPU() { if (m_pDevice) m_pDevice->WaitForGPU(); }
    std::mutex& GetMutex() { return m_mutex; }

    void PrepareCommandList() { if (m_pDevice) m_pDevice->PrepareCommandList(); }
    void SubmitCommandList() { if (m_pDevice) m_pDevice->SubmitCommandList(); }

private:
    // --- 렌더링 파이프라인 구축 관련 함수 ---
    void CreateRootSignature();     // 쉐이더 자원 바인딩 레이아웃 생성
    void CreatePipelineState();     // 그래픽 파이프라인 상태(PSO) 생성
    void CreateConstantBuffer();    // 상수 버퍼 생성

    void RenderGameObject(std::shared_ptr<class CGameObject> pObj, int& objIndex, class CLight* pLight);

private:
    // --- DX12 핵심 장치 ---
    std::unique_ptr<CDevice> m_pDevice;
    std::unique_ptr<CSwapChain> m_pMainSwapChain;
    std::unique_ptr<CSwapChain> m_pDebugSwapChain;
    std::unique_ptr<CGBuffer> m_pGBuffer;

    // --- 렌더링 파이프라인 구축 ---
    ComPtr<ID3D12RootSignature> m_rootSignatureDeferred;
    ComPtr<ID3D12RootSignature> m_rootSignatureLighting;
    ComPtr<ID3D12RootSignature> m_rootSignatureDebug;

    ComPtr<ID3D12PipelineState> m_pipelineStateDeferred;
    ComPtr<ID3D12PipelineState> m_pipelineStateLighting;
    ComPtr<ID3D12PipelineState> m_pipelineStateDebug;
    ComPtr<ID3D12PipelineState> m_pipelineStateGizmo; // 기즈모용 PSO (깊이 테스트 무시)
    
    std::unique_ptr<CConstantBuffer> m_pConstantBuffer; // 상수 버퍼 매니저

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
