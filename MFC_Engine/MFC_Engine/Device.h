#pragma once


class CDevice
{
public:
    CDevice();
    ~CDevice();

    bool Initialize(HWND hWnd, int width, int height);
    void Resize(int width, int height);
    
    void PrepareRender();
    void SubmitRender();
    void WaitForPreviousFrame();
    void WaitGPU();

    ID3D12Device* GetDevice() const { return m_pDevice.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return m_pCommandList.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return m_pCommandQueue.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() const { return m_pCommandAllocator.Get(); }
    
    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandle() const;
    
    // G-Buffer
    D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferRtvHandle(int index) const;
    ID3D12DescriptorHeap* GetGBufferSrvHeap() const { return m_pGBufferSrvHeap.Get(); }
    void TransitionGBuffersToRenderTarget();
    void TransitionGBuffersToPixelShaderResource();
    void ClearAndSetGBuffers();
    void SetMainRenderTarget();

    // Debug View
    bool InitializeDebugSwapChain(HWND hWnd, int width, int height);
    void ResizeDebugSwapChain(int width, int height);
    bool PrepareDebugRender();
    void SubmitDebugRender();
    D3D12_CPU_DESCRIPTOR_HANDLE GetDebugRtvHandle() const;
    
    const D3D12_VIEWPORT& GetViewport() const { return m_viewport; }
    const D3D12_RECT& GetScissorRect() const { return m_scissorRect; }
    
    UINT GetFrameIndex() const { return m_nFrameIndex; }

private:
    void CreateDevice();
    void CreateCommandQueue();
    void CreateSwapChain(HWND hWnd, int width, int height);
    void CreateDescriptorHeaps();
    void CreateRenderTargets();
    void CreateDepthStencilBuffer(int width, int height);
    void CreateGBuffers(int width, int height);

private:
    static const UINT FrameCount = 2;

    ComPtr<ID3D12Device> m_pDevice;
    ComPtr<ID3D12CommandQueue> m_pCommandQueue;
    ComPtr<IDXGISwapChain3> m_pSwapChain;
    
    ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
    UINT m_nRtvDescriptorSize;
    ComPtr<ID3D12Resource> m_pRenderTargets[FrameCount];

    ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;
    UINT m_nDsvDescriptorSize;
    ComPtr<ID3D12Resource> m_pDepthStencilBuffer;

    ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_pCommandList;

    UINT m_nFrameIndex;
    HANDLE m_hFenceEvent;
    ComPtr<ID3D12Fence> m_pFence;
    UINT64 m_nFenceValues[FrameCount];

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;

    // G-Buffer
    ComPtr<ID3D12Resource> m_pGBuffers[3];
    ComPtr<ID3D12DescriptorHeap> m_pGBufferRtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_pGBufferSrvHeap;

    // Debug View SwapChain
    ComPtr<IDXGISwapChain3> m_pDebugSwapChain;
    ComPtr<ID3D12Resource> m_pDebugRenderTargets[FrameCount];
    ComPtr<ID3D12DescriptorHeap> m_pDebugRtvHeap;
    UINT m_nDebugFrameIndex;
    D3D12_VIEWPORT m_debugViewport;
    D3D12_RECT m_debugScissorRect;
};
