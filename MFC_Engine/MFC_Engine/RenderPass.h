#pragma once
#include "pch.h"

/**
 * @struct RenderContext
 * @brief 렌더 패스 실행에 필요한 정보를 논리적으로 그룹화한 컨텍스트입니다.
 */
struct RenderContext
{
    // [Core] 공통 필수 정보
    ID3D12GraphicsCommandList* pCommandList;
    class CConstantBuffer* pCB;
    int nWidth;
    int nHeight;

    // [Scene] 씬 데이터 및 객체 정보
    struct SceneInfo {
        class CScene* pScene = nullptr;
        class CGameObject* pSelectedObj = nullptr;
    } scene;

    // [Resources] 렌더링 타겟 및 리소스
    struct RenderResources {
        class CSwapChain* pMainSwapChain = nullptr;
        class CGBuffer* pGBuffer = nullptr;
        class CGizmo* pGizmo = nullptr;
    } resources;
};

/**
 * @class CRenderPass
 * @brief 모든 렌더링 패스의 최상위 인터페이스 클래스입니다.
 */
class CRenderPass
{
public:
    virtual ~CRenderPass() = default;
    virtual void Initialize(ID3D12Device* pDevice) = 0;
    virtual void Execute(const RenderContext& context) = 0;

    ID3D12RootSignature* GetRootSignature() const { return m_rootSignature.Get(); }

protected:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
};
