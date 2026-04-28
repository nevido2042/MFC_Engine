#include "pch.h"
#include "PrimitiveGenerator.h"
#include <DirectXMath.h>

using namespace DirectX;

std::vector<Vertex> CPrimitiveGenerator::CreateCubeData()
{
    return std::vector<Vertex>
    {
        // 앞면 (Z-)
        { { -0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { -0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },

        // 뒷면 (Z+)
        { { -0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },

        // 윗면 (Y+)
        { { -0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },

        // 아랫면 (Y-)
        { { -0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },

        // 왼쪽면 (X-)
        { { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f, -0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
        { { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f, 1.0f, 1.0f } },

        // 오른쪽면 (X+)
        { {  0.5f,  0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
    };
}

std::vector<Vertex> CPrimitiveGenerator::CreatePlaneData()
{
    return std::vector<Vertex>
    {
        { { -0.5f, 0.0f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
        { {  0.5f, 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
        { { -0.5f, 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
        { { -0.5f, 0.0f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
        { {  0.5f, 0.0f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
        { {  0.5f, 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.8f, 0.8f, 0.8f, 1.0f } },
    };
}

std::vector<Vertex> CPrimitiveGenerator::CreateQuadData()
{
    return std::vector<Vertex>
    {
        { { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
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

            auto getNormal = [&](const XMFLOAT3& pos) {
                XMVECTOR p = XMLoadFloat3(&pos);
                p = XMVector3Normalize(p);
                XMFLOAT3 n;
                XMStoreFloat3(&n, p);
                return n;
            };

            XMFLOAT3 n1 = getNormal(v1);
            XMFLOAT3 n2 = getNormal(v2);
            XMFLOAT3 n3 = getNormal(v3);
            XMFLOAT3 n4 = getNormal(v4);

            XMFLOAT4 color = { 0.7f, 0.7f, 0.7f, 1.0f };

            vertices.push_back({ v1, n1, color });
            vertices.push_back({ v2, n2, color });
            vertices.push_back({ v3, n3, color });

            vertices.push_back({ v2, n2, color });
            vertices.push_back({ v4, n4, color });
            vertices.push_back({ v3, n3, color });
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

    auto getNormal = [&](float phi, float theta) {
        return XMFLOAT3(sinf(phi) * cosf(theta), cosf(phi), sinf(phi) * sinf(theta));
    };

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
            
            XMFLOAT3 n1 = getNormal(phi1, theta1); XMFLOAT3 n2 = getNormal(phi1, theta2);
            XMFLOAT3 n3 = getNormal(phi2, theta1); XMFLOAT3 n4 = getNormal(phi2, theta2);

            XMFLOAT4 color = { 0.7f, 0.7f, 0.7f, 1.0f };
            vertices.push_back({ v1, n1, color }); vertices.push_back({ v2, n2, color }); vertices.push_back({ v3, n3, color });
            vertices.push_back({ v2, n2, color }); vertices.push_back({ v4, n4, color }); vertices.push_back({ v3, n3, color });
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

            XMFLOAT3 n1 = getNormal(phi1, theta1); XMFLOAT3 n2 = getNormal(phi1, theta2);
            XMFLOAT3 n3 = getNormal(phi2, theta1); XMFLOAT3 n4 = getNormal(phi2, theta2);

            XMFLOAT4 color = { 0.7f, 0.7f, 0.7f, 1.0f };
            vertices.push_back({ v1, n1, color }); vertices.push_back({ v2, n2, color }); vertices.push_back({ v3, n3, color });
            vertices.push_back({ v2, n2, color }); vertices.push_back({ v4, n4, color }); vertices.push_back({ v3, n3, color });
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

        XMFLOAT3 n1 = { cosf(theta1), 0.0f, sinf(theta1) };
        XMFLOAT3 n2 = { cosf(theta2), 0.0f, sinf(theta2) };

        XMFLOAT4 color = { 0.7f, 0.7f, 0.7f, 1.0f };
        vertices.push_back({ v1, n1, color }); vertices.push_back({ v2, n2, color }); vertices.push_back({ v3, n1, color });
        vertices.push_back({ v2, n2, color }); vertices.push_back({ v4, n2, color }); vertices.push_back({ v3, n1, color });
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

void CPrimitiveGenerator::Initialize(ComPtr<ID3D12Device> device)
{
    if (!m_meshes.empty()) return;

    m_meshes[L"Cube"]    = CreateCubeMesh(device);
    m_meshes[L"Plane"]   = CreatePlaneMesh(device);
    m_meshes[L"Quad"]    = CreateQuadMesh(device);
    m_meshes[L"Sphere"]  = CreateSphereMesh(device);
    m_meshes[L"Capsule"] = CreateCapsuleMesh(device);
}

std::shared_ptr<CMesh> CPrimitiveGenerator::GetPrimitiveMesh(const std::wstring& name)
{
    auto it = m_meshes.find(name);
    if (it != m_meshes.end())
    {
        return it->second;
    }
    return nullptr;
}
