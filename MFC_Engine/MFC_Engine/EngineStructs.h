#pragma once
#include <DirectXMath.h>

/**
 * @struct Vertex
 * @brief 정점 데이터를 정의하는 구조체입니다.
 */
struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT4 color;
};

/**
 * @struct SceneConstantBuffer
 * @brief 쉐이더로 전달할 상수 데이터 구조체입니다.
 */
struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 matWVP;
    DirectX::XMFLOAT4X4 matWorld;
    DirectX::XMFLOAT4 objectColorID;
    DirectX::XMFLOAT4 meshColor; // 렌더링 시 사용할 색상 (기즈모 등)
    DirectX::XMFLOAT4 lightDir;
    DirectX::XMFLOAT4 lightColor;
    DirectX::XMFLOAT4 ambientColor;
    float padding[12]; // 256바이트 정렬 패딩
};
