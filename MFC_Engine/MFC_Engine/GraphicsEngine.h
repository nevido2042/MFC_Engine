#pragma once

#include "TimeManager.h"
#include <DirectXMath.h>
#include <mutex>

/**
 * @struct Vertex
 * @brief 삼각형의 정점 데이터를 정의하는 구조체입니다.
 */
struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

/**
 * @struct SceneConstantBuffer
 * @brief 쉐이더로 전달할 전역 데이터 구조체입니다 (256바이트 정렬 필요).
 */
struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 matRotation;
    float padding[48]; // 256바이트 패딩
};

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
    void Resize(int width, int height);
    float GetFPS() const { return m_timeManager.GetFPS(); }

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
    void CreateVertexBuffer();      // 정점 데이터 버퍼 생성
    void CreateConstantBuffer();    // 상수 버퍼 생성

    void WaitForPreviousFrame();

private:
    static const UINT FrameCount = 2;

    // --- DX12 핵심 객체 ---
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    // --- 렌더링 파이프라인 객체 ---
    ComPtr<ID3D12RootSignature> m_rootSignature; // 루트 시그니처
    ComPtr<ID3D12PipelineState> m_pipelineState; // 파이프라인 상태 객체(PSO)
    ComPtr<ID3D12Resource> m_vertexBuffer;      // 정점 버퍼 리소스
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView; // 정점 버퍼 뷰
    
    ComPtr<ID3D12Resource> m_constantBuffer;    // 상수 버퍼 리소스
    UINT8* m_pCbvDataBegin;                     // 상수 버퍼 매핑 포인터

    // --- 동기화 및 상태 변수 ---
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];

    UINT m_rtvDescriptorSize;
    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    bool m_isInitialized;
    int m_width;
    int m_height;

    // --- 매니저 객체 분리 ---
    CTimeManager m_timeManager;

    // --- 스레드 동기화 ---
    std::mutex m_mutex;
};
