#include "pch.h"
#include "PickingRenderTarget.h"
#include "d3dx12.h"

CPickingRenderTarget::CPickingRenderTarget() : m_width(0), m_height(0) {}
CPickingRenderTarget::~CPickingRenderTarget() {}

void CPickingRenderTarget::Initialize(ID3D12Device* device, int width, int height)
{
    m_width = width;
    m_height = height;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_READBACK;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = 256;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ThrowIfFailed(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_readbackBuffer)));
}

void CPickingRenderTarget::Resize(ID3D12Device* device, int width, int height)
{
    if (width <= 0 || height <= 0) return;
    m_width = width;
    m_height = height;
}

void CPickingRenderTarget::ReadPixelAsync(ID3D12GraphicsCommandList* commandList, int x, int y, ID3D12Resource* pSource)
{
    if (!pSource || x < 0 || y < 0 || x >= m_width || y >= m_height) return;

    // [CRITICAL] 0x80004005 해결을 위한 상태 전이 최적화
    // 소스 리소스가 현재 어떤 상태인지 알 수 없으므로, 
    // 여기서는 상태 전이를 수행하지 않고 호출자(PickingSystem)에게 위임하거나
    // GraphicsEngine의 현재 렌더링 상태를 따릅니다.
    
    // 일단 상태 전이를 제거하고 PickingSystem에서 일괄 관리하도록 변경합니다.
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
    src.pResource = pSource;
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
}

UINT CPickingRenderTarget::GetPickedID()
{
    if (!m_readbackBuffer) return 0;

    UINT8* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 4 };
    if (SUCCEEDED(m_readbackBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData))))
    {
        UINT8 r = mappedData[0];
        UINT8 g = mappedData[1];
        UINT8 b = mappedData[2];
        UINT8 a = mappedData[3];

        m_readbackBuffer->Unmap(0, nullptr);

        UINT id = r | (g << 8) | (b << 16);
        if (b == 255 && g == 0) id |= (a << 24);
        
        return id;
    }
    return 0;
}
