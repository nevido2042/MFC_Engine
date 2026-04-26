#pragma once
#include <DirectXMath.h>

/**
 * @struct Vertex
 * @brief 정점 데이터를 정의하는 구조체입니다.
 */
struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

/**
 * @struct SceneConstantBuffer
 * @brief 쉐이더로 전달할 상수 데이터 구조체입니다.
 */
struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 matWVP;
    float padding[48]; // 256바이트 패딩
};
