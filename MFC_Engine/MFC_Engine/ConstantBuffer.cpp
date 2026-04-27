#include "pch.h"
#include "ConstantBuffer.h"

CConstantBuffer::CConstantBuffer()
    : m_pCbvDataBegin(nullptr)
    , m_elementSize(0)
    , m_elementCount(0)
{
}

CConstantBuffer::~CConstantBuffer()
{
    if (m_constantBuffer != nullptr)
    {
        m_constantBuffer->Unmap(0, nullptr);
    }
}

void CConstantBuffer::Initialize(ID3D12Device* device, UINT elementSize, UINT elementCount)
{
    m_elementCount = elementCount;
    // DirectX 12 상수 버퍼는 256바이트 정렬이 필수입니다.
    m_elementSize = (elementSize + 255) & ~255;
    
    const UINT totalBufferSize = m_elementSize * m_elementCount;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = totalBufferSize;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ThrowIfFailed(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer)));

    // 상시 매핑 상태로 유지
    D3D12_RANGE readRange = { 0, 0 }; // CPU에서 읽지 않음
    ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
}

void CConstantBuffer::Update(UINT index, const void* pData, UINT dataSize)
{
    if (index >= m_elementCount) return;
    if (dataSize > m_elementSize) return;

    memcpy(m_pCbvDataBegin + (index * m_elementSize), pData, dataSize);
}

D3D12_GPU_VIRTUAL_ADDRESS CConstantBuffer::GetGPUVirtualAddress(UINT index) const
{
    if (index >= m_elementCount) return 0;
    return m_constantBuffer->GetGPUVirtualAddress() + (index * m_elementSize);
}
