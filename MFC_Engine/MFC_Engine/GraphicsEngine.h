#pragma once
#include "EngineStructs.h"
#include "TimeManager.h"
#include <mutex>

class CGraphicsEngine
{
public:
    CGraphicsEngine();
    ~CGraphicsEngine();

    // --- 핵심 인터페이스 ---
    bool Initialize(class CDevice* pDevice, int width, int height);
    
    // 컴포넌트 주입 (DI) - R-value 참조를 사용하여 호출부의 소멸자 참조 방지
    void SetSwapChain(std::unique_ptr<class CSwapChain>&& pSwapChain);
    void SetGBuffer(std::unique_ptr<class CGBuffer>&& pGBuffer);
    void SetImGuiManager(std::unique_ptr<class CImGuiManager>&& pImGui);

    void Render(std::shared_ptr<class CScene> pScene, std::shared_ptr<class CGameObject> pSelectedObj);
    void RenderDebugGBuffers();
    
    // 컴포넌트 주입 (DI)
    void SetPickingSystem(std::unique_ptr<class CPickingSystem>&& pPicking);
    void SetPrimitiveGenerator(std::unique_ptr<class CPrimitiveGenerator>&& pGenerator);

    class CPickingSystem* GetPickingSystem() { return m_pPickingSystem.get(); }
    class CPrimitiveGenerator* GetPrimitiveGenerator() { return m_pPrimitiveGenerator.get(); }
    
    // 디버그용 스왑체인 관련 (GBufferView 등에서 사용)
    bool InitializeDebugSwapChain(HWND hWnd, int width, int height);
    void ResizeDebugSwapChain(int width, int height);
    void Resize(int width, int height);

    float GetFPS() const;
    ID3D12Device* GetDevice();
    ID3D12RootSignature* GetRootSignature();

    void SetDebugViewActive(bool bActive) { m_bDebugViewActive = bActive; }
    bool IsDebugViewActive() const { return m_bDebugViewActive; }
    
    // --- 헬퍼 함수 ---
    ID3D12CommandQueue* GetCommandQueue();
    ID3D12CommandAllocator* GetCommandAllocator();
    ID3D12GraphicsCommandList* GetCommandList();
    ID3D12Resource* GetConstantBufferResource();
    UINT8* GetConstantBufferPtr();
    int GetWidth() const { return m_nWidth; }
    int GetHeight() const { return m_nHeight; }

    void PrepareCommandList();
    void SubmitCommandList();
    void WaitGPU();

    std::mutex& GetMutex() { return m_mutex; }
    ID3D12Resource* GetGBufferResource(int nIndex);

    // --- 렌더 패스 관리 ---
    void RegisterRenderPass(std::unique_ptr<class CRenderPass> pPass, bool bIsMainPass = true);

private:
    void CreateConstantBuffer();    // 상수 버퍼 생성

private:
    // --- DX12 핵심 장치 (외부 주입) ---
    class CDevice* m_pDevice = nullptr;
    std::unique_ptr<class CSwapChain> m_pMainSwapChain;
    std::unique_ptr<class CSwapChain> m_pDebugSwapChain;
    std::unique_ptr<class CGBuffer> m_pGBuffer;

    std::vector<std::unique_ptr<class CRenderPass>> m_mainRenderPasses;
    std::unique_ptr<class CDebugPass> m_pDebugPass;
    
    class CGeometryPass* m_pMainGeometryPass = nullptr; // RootSignature 참조용 포인터
    
    std::unique_ptr<class CConstantBuffer> m_pConstantBuffer; // 상수 버퍼 매니저
    std::unique_ptr<class CImGuiManager> m_pImGuiManager;

    std::unique_ptr<class CPickingSystem> m_pPickingSystem;
    std::unique_ptr<class CPrimitiveGenerator> m_pPrimitiveGenerator;

    // --- 동기화 및 상태 변수 ---
    bool m_bIsInitialized;
    bool m_bDebugViewActive = false;
    std::mutex m_mutex;

    CTimeManager m_timeManager;
    int m_nWidth;
    int m_nHeight;

    ComPtr<ID3D12RootSignature> m_pRootSignature;
};
