#pragma once
#include "EngineStructs.h"
#include <vector>
#include <memory>
#include <map>
#include <string>
#include "Mesh.h"

/**
 * @class CPrimitiveGenerator
 * @brief 기본 도형(기하학적 형태)의 데이터를 생성 및 관리하는 싱글톤 클래스입니다.
 */
class CPrimitiveGenerator
{
public:
    static CPrimitiveGenerator& GetInstance()
    {
        static CPrimitiveGenerator instance;
        return instance;
    }

    // --- 초기화 및 관리 ---
    void Initialize(ComPtr<ID3D12Device> device);
    std::shared_ptr<CMesh> GetPrimitiveMesh(const std::wstring& name);

    // --- 로우 데이터 생성 (유틸리티) ---
    static std::vector<Vertex> CreateCubeData();
    static std::vector<Vertex> CreatePlaneData();
    static std::vector<Vertex> CreateQuadData();
    static std::vector<Vertex> CreateSphereData(float radius = 0.5f, int sliceCount = 20, int stackCount = 20);
    static std::vector<Vertex> CreateCapsuleData(float radius = 0.5f, float height = 1.0f, int sliceCount = 20, int stackCount = 20);

    // --- 메쉬 객체 생성 (유틸리티) ---
    static std::shared_ptr<CMesh> CreateCubeMesh(ComPtr<ID3D12Device> device);
    static std::shared_ptr<CMesh> CreatePlaneMesh(ComPtr<ID3D12Device> device);
    static std::shared_ptr<CMesh> CreateQuadMesh(ComPtr<ID3D12Device> device);
    static std::shared_ptr<CMesh> CreateSphereMesh(ComPtr<ID3D12Device> device);
    static std::shared_ptr<CMesh> CreateCapsuleMesh(ComPtr<ID3D12Device> device);

private:
    CPrimitiveGenerator() = default;
    ~CPrimitiveGenerator() = default;

    std::map<std::wstring, std::shared_ptr<CMesh>> m_meshes;
};
