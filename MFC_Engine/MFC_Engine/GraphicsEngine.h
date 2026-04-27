#pragma once

#include "TimeManager.h"

class CScene;
class CPicking;
#include <DirectXMath.h>
#include <mutex>
#include <map>
#include <string>

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
    void Render();
    void Render(std::shared_ptr<CScene> pScene);
    void Resize(int width, int height);
    float GetFPS() const { return m_timeManager.GetFPS(); }
    
    UINT Pick(int x, int y);


private:
    // --- 초기화 내부 함수 ---
    void CreateDevice();
    void CreateCommandQueue();
    void CreateSwapChain(HWND hWnd, int width, int height);
    void CreateDescriptorHeaps();
    void CreateRenderTargets();
    void CreateCommandAllocator();
    
    // --- 렌더링 파이프라인 구축 관련 함수 ---
    void CreateRootSignature();     // 쉐이더 자원 바인딩 레이아웃 생성
    void CreatePipelineState();     // 그래픽 파이프라인 상태(PSO) 생성
    void CreateConstantBuffer();    // 상수 버퍼 생성
    void CreateDepthStencilBuffer(); // 깊이 버퍼 생성

    void RenderGameObject(std::shared_ptr<class CGameObject> pObj, int& objIndex);
    void WaitForPreviousFrame();

private:
    // --- DX12 핵심 장치 ---
    std::unique_ptr<CDevice> m_pDevice;

    // --- 렌더링 파이프라인 구축 ---
    ComPtr<ID3D12RootSignature> m_rootSignature; // 루트 시그니처
    ComPtr<ID3D12PipelineState> m_pipelineState; // 파이프라인 상태 객체(PSO)
    
    std::unique_ptr<CConstantBuffer> m_pConstantBuffer; // 상수 버퍼 매니저

    // --- 동기화 및 상태 변수 ---
    bool m_isInitialized;
    int m_width;
    int m_height;

    // --- 매니저 객체 분리 ---
    CTimeManager m_timeManager;

    // --- 스레드 동기화 ---
    std::mutex m_mutex;
};
