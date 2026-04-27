#pragma once
#include "PickingRenderTarget.h"
#include "Gizmo.h"

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
    
    UINT Pick(int x, int y, ID3D12Device* device, ID3D12CommandQueue* queue, ID3D12CommandAllocator* allocator, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin, int width, int height, std::shared_ptr<class CGameObject> pSelectedObj, class CGizmo* pGizmo);
    UINT GetPickedID();

private:
    void RenderPickingPass(std::shared_ptr<class CScene> pScene, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature, int width, int height, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin, std::shared_ptr<class CGameObject> pSelectedObj, class CGizmo* pGizmo);
    void RenderGameObjectForPicking(std::shared_ptr<class CGameObject> pObj, int& cbIndex, ID3D12GraphicsCommandList* commandList, int width, int height, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin);
    void RenderGizmoForPicking(std::shared_ptr<class CGameObject> pSelectedObj, class CGizmo* pGizmo, ID3D12GraphicsCommandList* commandList, int width, int height, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin);

private:
    ComPtr<ID3D12PipelineState> m_pickingPSO;
    std::unique_ptr<CPickingRenderTarget> m_pPickingRenderTarget;
};
