#pragma once
#include <DirectXMath.h>
#include <stdint.h>

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
 * @brief 쉐이더로 전달할 상수 데이터 구조체입니다. (Geometry Pass 전용)
 */
struct SceneConstantBuffer
{
    DirectX::XMFLOAT4X4 matWVP;
    DirectX::XMFLOAT4X4 matWorld;
    DirectX::XMFLOAT4 objectColorID;
    DirectX::XMFLOAT4 meshColor; 
    float padding[40]; // 256바이트 정렬 (96 bytes used, 160 bytes padding)
};

/**
 * @struct LightConstantBuffer
 * @brief 라이팅 패스에서 사용할 상수 데이터 구조체입니다.
 */
struct LightConstantBuffer
{
    DirectX::XMFLOAT4 lightDir;
    DirectX::XMFLOAT4 lightColor;
    DirectX::XMFLOAT4 ambientColor;
    float padding[52]; // 256바이트 정렬 (48 bytes used, 208 bytes padding)
};

/**
 * @struct DebugConstantBuffer
 * @brief GBuffer 디버그 시각화용 상수 데이터 구조체입니다.
 */
struct DebugConstantBuffer
{
    uint32_t nBufferCount;    // 전체 버퍼 개수
    uint32_t nGridCols;       // 가로 분할 수
    uint32_t nGridRows;       // 세로 분할 수
    uint32_t padding;
    uint32_t nBufferTypes[4]; // 각 버퍼의 타입 (0: Albedo, 1: Normal, 2: Position)
    float padding2[44];       // 256바이트 정렬
};
