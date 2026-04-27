#pragma once
#include "PickingRenderTarget.h"
#include <functional>

using Microsoft::WRL::ComPtr;

class CPickingSystem
{
public:
    static CPickingSystem& GetInstance()
    {
        static CPickingSystem instance;
        return instance;
    }

    void Initialize(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature, int width, int height);
    void Resize(ComPtr<ID3D12Device> device, int width, int height);
    
    // 기즈모 대신 범용적인 추가 렌더링 콜백을 받음
    using PickingOverlayFunc = std::function<void(ID3D12GraphicsCommandList*, ID3D12Resource*, UINT8*)>;
    UINT Pick(int x, int y, class CGraphicsEngine* pEngine, PickingOverlayFunc overlayFunc = nullptr);

private:
    void RenderPickingPass(std::shared_ptr<class CScene> pScene, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature, int width, int height, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin, PickingOverlayFunc overlayFunc);
    void RenderGameObjectForPicking(std::shared_ptr<class CGameObject> pObj, int& cbIndex, ID3D12GraphicsCommandList* commandList, int width, int height, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin);

private:
    ComPtr<ID3D12PipelineState> m_pickingPSO;
    std::unique_ptr<CPickingRenderTarget> m_pPickingRenderTarget;
};
