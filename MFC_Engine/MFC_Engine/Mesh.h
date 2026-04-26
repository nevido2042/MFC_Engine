#pragma once
#include "EngineStructs.h"
#include <vector>
#include <wrl.h>
#include <d3d12.h>

using namespace Microsoft::WRL;

/**
 * @class CMesh
 * @brief 정점 버퍼와 인덱스 버퍼를 관리하고 렌더링 명령을 수행하는 클래스입니다.
 */
class CMesh
{
public:
    CMesh();
    ~CMesh();

    /**
     * @brief 정점 데이터를 기반으로 메쉬를 초기화합니다.
     * @param device DX12 장치
     * @param vertices 정점 데이터 배열
     */
    void Initialize(ComPtr<ID3D12Device> device, const std::vector<Vertex>& vertices);

    /**
     * @brief 메쉬를 화면에 그리기 위한 설정을 수행합니다.
     * @param commandList 명령 리스트
     */
    void Render(ComPtr<ID3D12GraphicsCommandList> commandList);

    UINT GetVertexCount() const { return m_vertexCount; }

private:
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    UINT m_vertexCount;
};
