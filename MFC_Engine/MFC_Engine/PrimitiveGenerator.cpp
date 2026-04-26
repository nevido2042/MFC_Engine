#include "pch.h"
#include "PrimitiveGenerator.h"

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
