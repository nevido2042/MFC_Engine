#include "pch.h"
#include "PickingSystem.h"
#include "GraphicsEngine.h"
#include "d3dx12.h"

void CPickingSystem::Initialize(ComPtr<ID3D12Device> device, ID3D12RootSignature* rootSignature, int width, int height)
{
    m_nWidth = width;
    m_nHeight = height;
    CreateReadbackBuffer(device.Get());
}

void CPickingSystem::Resize(ComPtr<ID3D12Device> device, int width, int height)
{
    m_nWidth = width;
    m_nHeight = height;
}

void CPickingSystem::CreateReadbackBuffer(ID3D12Device* device)
{
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_READBACK;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = 256; // 256-byte alignment
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
        IID_PPV_ARGS(&m_pReadbackBuffer)));
}

UINT CPickingSystem::Pick(int x, int y, CGraphicsEngine* pEngine)
{
    if (!pEngine || !m_pReadbackBuffer) return 0;
    if (x < 0 || y < 0 || x >= m_nWidth || y >= m_nHeight) return 0;

    std::lock_guard<std::mutex> lock(pEngine->GetMutex());

    auto pCmdList = pEngine->GetCommandList();
    auto pIDBuffer = pEngine->GetGBufferResource(3); 
    if (!pIDBuffer) return 0;

    pEngine->PrepareCommandList();

    // 1. Transition ID Buffer to COPY_SOURCE
    pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pIDBuffer,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));

    // 2. Copy Texture Region (Pixel) to Readback Buffer
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
    footprint.Footprint.Width = 1;
    footprint.Footprint.Height = 1;
    footprint.Footprint.Depth = 1;
    footprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    footprint.Footprint.RowPitch = 256;

    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource = m_pReadbackBuffer.Get();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dst.PlacedFootprint = footprint;

    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource = pIDBuffer;
    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex = 0;

    D3D12_BOX box;
    box.left = x; box.right = x + 1;
    box.top = y; box.bottom = y + 1;
    box.front = 0; box.back = 1;

    pCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);

    // 3. Transition back
    pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pIDBuffer,
        D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

    pEngine->SubmitCommandList();
    pEngine->WaitGPU();
    pEngine->PrepareCommandList();

    // 4. Map and Read Result
    UINT8* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 4 };
    UINT pickedID = 0;

    if (SUCCEEDED(m_pReadbackBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedData))))
    {
        // Decode ID (R | G << 8 | B << 16)
        pickedID = mappedData[0] | (mappedData[1] << 8) | (mappedData[2] << 16);
        m_pReadbackBuffer->Unmap(0, nullptr);
    }

    return pickedID;
}
