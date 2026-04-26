#pragma once
#include "EngineStructs.h"
#include <vector>
#include <memory>
#include "Mesh.h"

/**
 * @class CPrimitiveGenerator
 * @brief 기본 도형(기하학적 형태)의 데이터를 생성하는 유틸리티 클래스입니다.
 */
class CPrimitiveGenerator
{
public:
    // --- 로우 데이터 생성 ---
    static std::vector<Vertex> CreateCubeData();
    static std::vector<Vertex> CreatePlaneData();

    // --- 메쉬 객체 생성 ---
    static std::shared_ptr<CMesh> CreateCubeMesh(ComPtr<ID3D12Device> device);
    static std::shared_ptr<CMesh> CreatePlaneMesh(ComPtr<ID3D12Device> device);
};
