#pragma once


class CDevice
{
public:
    CDevice();
    ~CDevice();

    bool Initialize();

    ID3D12Device* GetDevice() const { return m_pDevice.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return m_pCommandQueue.Get(); }
    ID3D12CommandAllocator* GetCommandAllocator() const { return m_pCommandAllocator.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return m_pCommandList.Get(); }

    void PrepareCommandList();
    void SubmitCommandList();
    void WaitForGPU();

private:
    void CreateDevice();
    void CreateCommandObjects();
    void CreateFenceAndEvent();

private:
    ComPtr<ID3D12Device> m_pDevice;
    ComPtr<ID3D12CommandQueue> m_pCommandQueue;
    ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_pCommandList;

    HANDLE m_hFenceEvent;
    ComPtr<ID3D12Fence> m_pFence;
    UINT64 m_nFenceValue;
};
