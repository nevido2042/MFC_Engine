#include "pch.h"
#include "Device.h"

CDevice::CDevice()
    : m_nFenceValue(0)
    , m_hFenceEvent(nullptr)
{
}

CDevice::~CDevice()
{
    WaitForGPU();
    if (m_hFenceEvent)
    {
        CloseHandle(m_hFenceEvent);
    }
}

bool CDevice::Initialize()
{
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
    }
#endif

    CreateDevice();
    CreateCommandObjects();
    CreateFenceAndEvent();

    return true;
}

void CDevice::CreateDevice()
{
    UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    ComPtr<IDXGIAdapter1> hardwareAdapter;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &hardwareAdapter); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        hardwareAdapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

        if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
        {
            break;
        }
    }

    ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice)));
}

void CDevice::CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));
    ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator)));
    ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList)));

    // Close command list initially
    m_pCommandList->Close();
}

void CDevice::CreateFenceAndEvent()
{
    ThrowIfFailed(m_pDevice->CreateFence(m_nFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
    m_nFenceValue++;

    m_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_hFenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }
}

void CDevice::PrepareCommandList()
{
    m_pCommandAllocator->Reset();
    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);
}

void CDevice::SubmitCommandList()
{
    ThrowIfFailed(m_pCommandList->Close());

    ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void CDevice::WaitForGPU()
{
    const UINT64 fence = m_nFenceValue;
    ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), fence));
    m_nFenceValue++;

    if (m_pFence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_pFence->SetEventOnCompletion(fence, m_hFenceEvent));
        WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
}
