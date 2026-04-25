#pragma once

/**
 * @class CGraphicsEngine
 * @brief DirectX 12 기반의 그래픽 렌더링 핵심 엔진 클래스입니다.
 * 
 * 이 클래스는 DX12 장치(Device) 생성, 스왑 체인 관리, 커맨드 리스트 기록 및 
 * GPU 동기화를 담당하며, MFC View와 연결되어 실시간 렌더링을 수행합니다.
 * 유니티와 같은 엔진의 로우레벨 렌더러 역할을 수행합니다.
 */
class CGraphicsEngine
{
public:
    CGraphicsEngine();
    ~CGraphicsEngine();

    // --- 핵심 인터페이스 ---
    
    /** @brief 엔진 초기화 (장치, 큐, 스왑체인 등 생성) */
    bool Initialize(HWND hWnd, int width, int height);
    
    /** @brief 화면 그리기 명령 기록 및 제출 */
    void Render();
    
    /** @brief 윈도우 크기 변경 시 스왑체인 버퍼 재구축 */
    void Resize(int width, int height);

private:
    // --- 초기화 내부 함수 ---
    void CreateDevice();            // GPU 장치 생성
    void CreateCommandQueue();      // 명령 큐 생성
    void CreateSwapChain(HWND hWnd, int width, int height); // 스왑체인 생성
    void CreateDescriptorHeaps();   // 렌더 타겟 뷰(RTV) 힙 생성
    void CreateRenderTargets();     // 후면 버퍼 리소스 생성
    void CreateCommandAllocator();  // 명령 할당자 생성
    void WaitForPreviousFrame();    // GPU 작업 완료 대기 (동기화)

private:
    static const UINT FrameCount = 2; // 더블 버퍼링 설정

    // --- DX12 핵심 객체 ---
    ComPtr<ID3D12Device> m_device;               // 물리적 GPU와 상호작용하는 장치
    ComPtr<ID3D12CommandQueue> m_commandQueue;   // GPU에 명령을 전달하는 통로
    ComPtr<IDXGISwapChain3> m_swapChain;         // 화면 전환을 관리하는 스왑체인
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;      // 렌더 타겟 서술자 저장소
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount]; // 실제 그려질 메모리 버퍼
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;  // 명령이 저장될 메모리 영역
    ComPtr<ID3D12GraphicsCommandList> m_commandList;    // 렌더링 명령을 기록하는 리스트

    // --- 동기화 및 상태 변수 ---
    UINT m_frameIndex;                  // 현재 사용 중인 후면 버퍼 인덱스
    HANDLE m_fenceEvent;                // GPU 대기용 이벤트 핸들
    ComPtr<ID3D12Fence> m_fence;        // CPU와 GPU 간의 동기화를 위한 펜스
    UINT64 m_fenceValues[FrameCount];   // 각 프레임별 동기화 값

    UINT m_rtvDescriptorSize;           // RTV 서술자의 메모리 크기
    D3D12_VIEWPORT m_viewport;          // 렌더링될 화면 영역
    D3D12_RECT m_scissorRect;           // 렌더링 컷팅 영역

    bool m_isInitialized;               // 엔진 초기화 여부 플래그
    int m_width;                        // 화면 너비
    int m_height;                       // 화면 높이
};
