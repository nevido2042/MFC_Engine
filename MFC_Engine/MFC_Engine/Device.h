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

    ID3D12Device* GetDevice() const { return m_device.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() const { return m_commandAllocator.Get(); }
    
    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvHandle() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandle() const;
    
    const D3D12_VIEWPORT& GetViewport() const { return m_viewport; }
    const D3D12_RECT& GetScissorRect() const { return m_scissorRect; }
    
    UINT GetFrameIndex() const { return m_frameIndex; }

private:
    void CreateDevice();
    void CreateCommandQueue();
    void CreateSwapChain(HWND hWnd, int width, int height);
    void CreateDescriptorHeaps();
    void CreateRenderTargets();
    void CreateDepthStencilBuffer(int width, int height);

private:
    static const UINT FrameCount = 2;

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

    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValues[FrameCount];

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;
};
