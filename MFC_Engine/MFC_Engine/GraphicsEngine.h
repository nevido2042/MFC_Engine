#pragma once

// 인라인 전방 선언 사용으로 외부 클래스 의존성 제거
#include "TimeManager.h"
#include "PrimitiveGenerator.h"
#include "ConstantBuffer.h"
#include "Device.h"

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
    void Resize(int width, int height);
    float GetFPS() const { return m_timeManager.GetFPS(); }
    ID3D12Device* GetDevice() { return m_pDevice ? m_pDevice->GetDevice() : nullptr; }
    
    // --- 리소스 접근자 (CPickingSystem 등에서 사용) ---
    ID3D12CommandQueue* GetCommandQueue() { return m_pDevice->GetCommandQueue(); }
    ID3D12CommandAllocator* GetCommandAllocator() { return m_pDevice->GetCommandAllocator(); }
    ID3D12GraphicsCommandList* GetCommandList() { return m_pDevice->GetCommandList(); }
    ID3D12RootSignature* GetRootSignature() { return m_rootSignature.Get(); }
    ID3D12Resource* GetConstantBufferResource() { return m_pConstantBuffer->GetResource(); }
    UINT8* GetConstantBufferPtr() { return m_pConstantBuffer->GetMappedData(); }
    int GetWidth() const { return m_nWidth; }
    int GetHeight() const { return m_nHeight; }

    void WaitGPU() { if (m_pDevice) m_pDevice->WaitGPU(); }

private:
    // --- 초기화 내부 함수 ---
    void CreateDevice();
    void CreateCommandQueue();
    void CreateSwapChain(HWND hWnd, int width, int height);
    void CreateDescriptorHeaps();
    void CreateRenderTargets();
    
    // --- 렌더링 파이프라인 구축 관련 함수 ---
    void CreateRootSignature();     // 쉐이더 자원 바인딩 레이아웃 생성
    void CreatePipelineState();     // 그래픽 파이프라인 상태(PSO) 생성
    void CreateConstantBuffer();    // 상수 버퍼 생성

    void RenderGameObject(std::shared_ptr<class CGameObject> pObj, int& objIndex, class CLight* pLight);
    void RenderGizmo(class CGizmo* pGizmo, std::shared_ptr<class CGameObject> pSelectedObj);
    void WaitForPreviousFrame();

private:
    // --- DX12 핵심 장치 ---
    std::unique_ptr<CDevice> m_pDevice;

    // --- 렌더링 파이프라인 구축 ---
    ComPtr<ID3D12RootSignature> m_rootSignature; // 루트 시그니처
    ComPtr<ID3D12PipelineState> m_pipelineState; // 기본 파이프라인 상태 객체(PSO)
    ComPtr<ID3D12PipelineState> m_pipelineStateGizmo; // 기즈모용 PSO (깊이 테스트 무시)
    
    std::unique_ptr<CConstantBuffer> m_pConstantBuffer; // 상수 버퍼 매니저

    // --- 동기화 및 상태 변수 ---
    bool m_bIsInitialized;
    int m_nWidth;
    int m_nHeight;

    // --- 매니저 객체 분리 ---
    CTimeManager m_timeManager;

    // --- 스레드 동기화 ---
    std::mutex m_mutex;
};
