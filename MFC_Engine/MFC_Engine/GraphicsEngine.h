#pragma once

#include "TimeManager.h"
#include "Camera.h"

class CScene;
#include <DirectXMath.h>
#include <mutex>
#include <map>
#include <string>

#include "PrimitiveGenerator.h"

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

    // --- 카메라 제어 ---
    void MoveCamera(float forward, float right, float up, float deltaTime);
    void RotateCamera(float pitch, float yaw);

    // --- 메쉬 관리 ---
    std::shared_ptr<class CMesh> GetPrimitiveMesh(const std::wstring& name);

private:
    // --- 초기화 내부 함수 ---
    void CreateDevice();
    void CreateCommandQueue();
    void CreateSwapChain(HWND hWnd, int width, int height);
    void CreateDescriptorHeaps();
    void CreateRenderTargets();
    void CreateCommandAllocator();
    
    // --- 삼각형 렌더링 관련 추가 함수 ---
    void CreateRootSignature();     // 쉐이더 자원 바인딩 레이아웃 생성
    void CreatePipelineState();     // 그래픽 파이프라인 상태(PSO) 생성
    void CreatePrimitiveMeshes();   // 기본 도형 메쉬 생성
    void CreateConstantBuffer();    // 상수 버퍼 생성
    void CreateDepthStencilBuffer(); // 깊이 버퍼 생성

    void WaitForPreviousFrame();

private:
    static const UINT FrameCount = 2;

    // --- DX12 핵심 객체 ---
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<IDXGISwapChain3> m_swapChain;
    
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    UINT m_rtvDescriptorSize;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];

    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    UINT m_dsvDescriptorSize;
    ComPtr<ID3D12Resource> m_depthStencilBuffer;

    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // --- 렌더링 파이프라인 객체 ---
    ComPtr<ID3D12RootSignature> m_rootSignature; // 루트 시그니처
    ComPtr<ID3D12PipelineState> m_pipelineState; // 파이프라인 상태 객체(PSO)
    
    // 메쉬 캐시
    std::map<std::wstring, std::shared_ptr<class CMesh>> m_meshes;
    
    ComPtr<ID3D12Resource> m_constantBuffer;    // 상수 버퍼 리소스
    UINT8* m_pCbvDataBegin;                     // 상수 버퍼 매핑 포인터

    // --- 동기화 및 상태 변수 ---
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    bool m_isInitialized;
    int m_width;
    int m_height;

    // --- 카메라 데이터 ---
    CCamera m_camera;

    // --- 매니저 객체 분리 ---
    CTimeManager m_timeManager;

    // --- 스레드 동기화 ---
    std::mutex m_mutex;
};
