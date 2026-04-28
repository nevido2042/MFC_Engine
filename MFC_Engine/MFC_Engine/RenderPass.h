#pragma once

#include "pch.h"

struct RenderContext
{
    ID3D12GraphicsCommandList* pCommandList;
    class CScene* pScene;
    class CSwapChain* pMainSwapChain;
    class CGBuffer* pGBuffer;
    class CConstantBuffer* pCB;
    class CGizmo* pGizmo;
    class CGameObject* pSelectedObj;
    int nWidth;
    int nHeight;
};

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
