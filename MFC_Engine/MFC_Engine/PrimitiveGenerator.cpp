#include "pch.h"
#include "PrimitiveGenerator.h"
#include <DirectXMath.h>

using namespace DirectX;

std::vector<Vertex> CPrimitiveGenerator::CreateCubeData()
{
    return std::vector<Vertex>
    {
        // 앞면 (Red)
        { { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },

        // 뒷면 (Green)
        { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },

        // 윗면 (Blue)
        { { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },

        // 아랫면 (Yellow)
        { { -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },

        // 왼쪽면 (Magenta)
        { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },

        // 오른쪽면 (Cyan)
        { {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
    };
}

std::vector<Vertex> CPrimitiveGenerator::CreatePlaneData()
{
    return std::vector<Vertex>
    {
        { { -0.5f, 0.0f,  0.5f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
        { {  0.5f, 0.0f, -0.5f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
        { { -0.5f, 0.0f, -0.5f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
        { { -0.5f, 0.0f,  0.5f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
        { {  0.5f, 0.0f,  0.5f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
        { {  0.5f, 0.0f, -0.5f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
    };
}

std::vector<Vertex> CPrimitiveGenerator::CreateQuadData()
{
    return std::vector<Vertex>
    {
        { { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
    };
}

std::vector<Vertex> CPrimitiveGenerator::CreateSphereData(float radius, int sliceCount, int stackCount)
{
    std::vector<Vertex> vertices;
    float phiStep = XM_PI / stackCount;
    float thetaStep = 2.0f * XM_PI / sliceCount;

    for (int i = 0; i < stackCount; ++i)
    {
        float phi1 = i * phiStep;
        float phi2 = (i + 1) * phiStep;

        for (int j = 0; j < sliceCount; ++j)
        {
            float theta1 = j * thetaStep;
            float theta2 = (j + 1) * thetaStep;

            auto getPos = [&](float phi, float theta) {
                return XMFLOAT3(
                    radius * sinf(phi) * cosf(theta),
                    radius * cosf(phi),
                    radius * sinf(phi) * sinf(theta)
                );
            };

            XMFLOAT3 v1 = getPos(phi1, theta1);
            XMFLOAT3 v2 = getPos(phi1, theta2);
            XMFLOAT3 v3 = getPos(phi2, theta1);
            XMFLOAT3 v4 = getPos(phi2, theta2);

            XMFLOAT4 color = { 0.7f, 0.7f, 0.7f, 1.0f };

            vertices.push_back({ v1, color });
            vertices.push_back({ v2, color });
            vertices.push_back({ v3, color });

            vertices.push_back({ v2, color });
            vertices.push_back({ v4, color });
            vertices.push_back({ v3, color });
        }
    }
    return vertices;
}

std::vector<Vertex> CPrimitiveGenerator::CreateCapsuleData(float radius, float height, int sliceCount, int stackCount)
{
    std::vector<Vertex> vertices;
    float halfHeight = height * 0.5f;
    float phiStep = XM_PI / stackCount;
    float thetaStep = 2.0f * XM_PI / sliceCount;

    // 상단 반구
    for (int i = 0; i < stackCount / 2; ++i)
    {
        float phi1 = i * phiStep;
        float phi2 = (i + 1) * phiStep;
        for (int j = 0; j < sliceCount; ++j)
        {
            float theta1 = j * thetaStep;
            float theta2 = (j + 1) * thetaStep;
            auto getPos = [&](float phi, float theta) {
                return XMFLOAT3(radius * sinf(phi) * cosf(theta), radius * cosf(phi) + halfHeight, radius * sinf(phi) * sinf(theta));
            };
            XMFLOAT3 v1 = getPos(phi1, theta1); XMFLOAT3 v2 = getPos(phi1, theta2);
            XMFLOAT3 v3 = getPos(phi2, theta1); XMFLOAT3 v4 = getPos(phi2, theta2);
            XMFLOAT4 color = { 0.7f, 0.7f, 0.7f, 1.0f };
            vertices.push_back({ v1, color }); vertices.push_back({ v2, color }); vertices.push_back({ v3, color });
            vertices.push_back({ v2, color }); vertices.push_back({ v4, color }); vertices.push_back({ v3, color });
        }
    }

    // 하단 반구
    for (int i = stackCount / 2; i < stackCount; ++i)
    {
        float phi1 = i * phiStep;
        float phi2 = (i + 1) * phiStep;
        for (int j = 0; j < sliceCount; ++j)
        {
            float theta1 = j * thetaStep;
            float theta2 = (j + 1) * thetaStep;
            auto getPos = [&](float phi, float theta) {
                return XMFLOAT3(radius * sinf(phi) * cosf(theta), radius * cosf(phi) - halfHeight, radius * sinf(phi) * sinf(theta));
            };
            XMFLOAT3 v1 = getPos(phi1, theta1); XMFLOAT3 v2 = getPos(phi1, theta2);
            XMFLOAT3 v3 = getPos(phi2, theta1); XMFLOAT3 v4 = getPos(phi2, theta2);
            XMFLOAT4 color = { 0.7f, 0.7f, 0.7f, 1.0f };
            vertices.push_back({ v1, color }); vertices.push_back({ v2, color }); vertices.push_back({ v3, color });
            vertices.push_back({ v2, color }); vertices.push_back({ v4, color }); vertices.push_back({ v3, color });
        }
    }

    // 중간 실린더
    for (int j = 0; j < sliceCount; ++j)
    {
        float theta1 = j * thetaStep;
        float theta2 = (j + 1) * thetaStep;
        XMFLOAT3 v1 = { radius * cosf(theta1),  halfHeight, radius * sinf(theta1) };
        XMFLOAT3 v2 = { radius * cosf(theta2),  halfHeight, radius * sinf(theta2) };
        XMFLOAT3 v3 = { radius * cosf(theta1), -halfHeight, radius * sinf(theta1) };
        XMFLOAT3 v4 = { radius * cosf(theta2), -halfHeight, radius * sinf(theta2) };
        XMFLOAT4 color = { 0.7f, 0.7f, 0.7f, 1.0f };
        vertices.push_back({ v1, color }); vertices.push_back({ v2, color }); vertices.push_back({ v3, color });
        vertices.push_back({ v2, color }); vertices.push_back({ v4, color }); vertices.push_back({ v3, color });
    }

    return vertices;
}

std::shared_ptr<CMesh> CPrimitiveGenerator::CreateCubeMesh(ComPtr<ID3D12Device> device)
{
    auto mesh = std::make_shared<CMesh>();
    mesh->Initialize(device, CreateCubeData());
    return mesh;
}

std::shared_ptr<CMesh> CPrimitiveGenerator::CreatePlaneMesh(ComPtr<ID3D12Device> device)
{
    auto mesh = std::make_shared<CMesh>();
    mesh->Initialize(device, CreatePlaneData());
    return mesh;
}

std::shared_ptr<CMesh> CPrimitiveGenerator::CreateQuadMesh(ComPtr<ID3D12Device> device)
{
    auto mesh = std::make_shared<CMesh>();
    mesh->Initialize(device, CreateQuadData());
    return mesh;
}

std::shared_ptr<CMesh> CPrimitiveGenerator::CreateSphereMesh(ComPtr<ID3D12Device> device)
{
    auto mesh = std::make_shared<CMesh>();
    mesh->Initialize(device, CreateSphereData());
    return mesh;
}

std::shared_ptr<CMesh> CPrimitiveGenerator::CreateCapsuleMesh(ComPtr<ID3D12Device> device)
{
    auto mesh = std::make_shared<CMesh>();
    mesh->Initialize(device, CreateCapsuleData());
    return mesh;
}
