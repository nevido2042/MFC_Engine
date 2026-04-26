#include "pch.h"
#include "Picking.h"
#include "d3dx12.h"

#ifndef ThrowIfFailed
#define ThrowIfFailed(x) \
{ \
    HRESULT hr__ = (x); \
    if(FAILED(hr__)) { throw std::exception(); } \
}
#endif

CPicking::CPicking() : m_width(0), m_height(0) {}
CPicking::~CPicking() {}

void CPicking::Initialize(ID3D12Device* device, int width, int height)
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

    // Minimum texture pitch alignment is 256 bytes
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_READBACK;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Alignment = 0;
    resDesc.Width = 256;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ThrowIfFailed(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_readbackBuffer)));

    Resize(device, width, height);
}

void CPicking::Resize(ID3D12Device* device, int width, int height)
{
    if (width <= 0 || height <= 0) return;
    m_width = width;
    m_height = height;

    m_pickingRT.Reset();
    m_pickingDSV.Reset();

    // Create RT
    D3D12_RESOURCE_DESC rtDesc = {};
    rtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rtDesc.Alignment = 0;
    rtDesc.Width = width;
    rtDesc.Height = height;
    rtDesc.DepthOrArraySize = 1;
    rtDesc.MipLevels = 1;
    rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtDesc.SampleDesc.Count = 1;
    rtDesc.SampleDesc.Quality = 0;
    rtDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    rtDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 0.0f;

    D3D12_HEAP_PROPERTIES heapPropsDef = {};
    heapPropsDef.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapPropsDef.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapPropsDef.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapPropsDef.CreationNodeMask = 1;
    heapPropsDef.VisibleNodeMask = 1;
    ThrowIfFailed(device->CreateCommittedResource(
        &heapPropsDef,
        D3D12_HEAP_FLAG_NONE,
        &rtDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearValue,
        IID_PPV_ARGS(&m_pickingRT)));

    device->CreateRenderTargetView(m_pickingRT.Get(), nullptr, m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    // Create DSV
    D3D12_RESOURCE_DESC dsDesc = {};
    dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    dsDesc.Alignment = 0;
    dsDesc.Width = width;
    dsDesc.Height = height;
    dsDesc.DepthOrArraySize = 1;
    dsDesc.MipLevels = 1;
    dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsDesc.SampleDesc.Count = 1;
    dsDesc.SampleDesc.Quality = 0;
    dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    
    D3D12_CLEAR_VALUE dsClear = {};
    dsClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsClear.DepthStencil.Depth = 1.0f;
    dsClear.DepthStencil.Stencil = 0;

    ThrowIfFailed(device->CreateCommittedResource(
        &heapPropsDef,
        D3D12_HEAP_FLAG_NONE,
        &dsDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &dsClear,
        IID_PPV_ARGS(&m_pickingDSV)));

    device->CreateDepthStencilView(m_pickingDSV.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void CPicking::ReadPixelAsync(ID3D12GraphicsCommandList* commandList, int x, int y)
{
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) return;

    // Transition RT to COPY_SOURCE
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pickingRT.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE));

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    footprint.Footprint.Width = 1;
    footprint.Footprint.Height = 1;
    footprint.Footprint.Depth = 1;
    footprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    footprint.Footprint.RowPitch = 256;
    footprint.Offset = 0;

    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource = m_readbackBuffer.Get();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dst.PlacedFootprint = footprint;

    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource = m_pickingRT.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex = 0;

    D3D12_BOX box;
    box.left = x;
    box.right = x + 1;
    box.top = y;
    box.bottom = y + 1;
    box.front = 0;
    box.back = 1;

    commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);

    // Transition RT back to RENDER_TARGET
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pickingRT.Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));
}

UINT CPicking::GetPickedID()
{
    UINT8* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 4 };
    if (SUCCEEDED(m_readbackBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData))))
    {
        UINT8 r = mappedData[0];
        UINT8 g = mappedData[1];
        UINT8 b = mappedData[2];
        UINT8 a = mappedData[3];

        m_readbackBuffer->Unmap(0, nullptr);

        // Decode ID: assuming R is least significant byte
        UINT id = r | (g << 8) | (b << 16) | (a << 24);
        return id;
    }
    return 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE CPicking::GetRTV() const
{
    return m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE CPicking::GetDSV() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}
