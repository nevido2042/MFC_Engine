#include "pch.h"
#include "Gizmo.h"
#include "GameObject.h"
#include "Transform.h"
#include "SceneManager.h"
#include "Camera.h"
#include "PrimitiveGenerator.h"
#include "Mesh.h"
#include "ConstantBuffer.h"
#include "EngineStructs.h"

CGizmo::CGizmo()
{
}

CGizmo::~CGizmo()
{
}

void CGizmo::Initialize(ID3D12Device* pDevice)
{
    m_pDevice = pDevice;
}

void CGizmo::Render(ID3D12GraphicsCommandList* pCommandList, ID3D12RootSignature* pRootSignature, CGameObject* pSelectedObj, int nWidth, int nHeight, CConstantBuffer* pCB)
{
    DrawAxes(pCommandList, pSelectedObj, nWidth, nHeight, false, pCB, nullptr, nullptr);
}

void CGizmo::RenderForPicking(ID3D12GraphicsCommandList* pCommandList, CGameObject* pSelectedObj, int nWidth, int nHeight, ID3D12Resource* pConstantBuffer, UINT8* pCbvDataBegin)
{
    DrawAxes(pCommandList, pSelectedObj, nWidth, nHeight, true, nullptr, pConstantBuffer, pCbvDataBegin);
}

void CGizmo::DrawAxes(ID3D12GraphicsCommandList* pCommandList, CGameObject* pSelectedObj, int nWidth, int nHeight, bool bForPicking, CConstantBuffer* pCB, ID3D12Resource* pRawCB, UINT8* pRawData)
{
    if (!pSelectedObj || !pCommandList)
    {
        return;
    }

    auto pTransform = pSelectedObj->GetTransform();
    if (!pTransform)
    {
        return;
    }

    // 선택된 오브젝트의 월드 위치 가져오기
    DirectX::XMVECTOR vPos, vRot, vScale;
    DirectX::XMMatrixDecompose(&vScale, &vRot, &vPos, pTransform->GetWorldMatrix());

    DirectX::XMMATRIX matView;
    {
        std::lock_guard<std::mutex> camLock(CSceneManager::GetInstance().GetCameraMutex());
        matView = CSceneManager::GetInstance().GetEditorCamera().GetViewMatrix();
    }

    float fAspectRatio = static_cast<float>(nWidth) / static_cast<float>(nHeight);
    DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, fAspectRatio, 0.1f, 1000.0f);

    auto pCubeMesh = CPrimitiveGenerator::GetInstance().GetPrimitiveMesh(L"Cube");

    if (!pCubeMesh)
    {
        return;
    }

    AxisInfo axes[] = {
        { { 1.0f, 0.05f, 0.05f }, { 0.5f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, 0xFF0001 }, // X-Axis (Red)
        { { 0.05f, 1.0f, 0.05f }, { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, 0xFF0002 }, // Y-Axis (Green)
        { { 0.05f, 0.05f, 1.0f }, { 0.0f, 0.0f, 0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, 0xFF0003 }  // Z-Axis (Blue)
    };

    int nGizmoIdxStart = 1000;

    for (int i = 0; i < 3; ++i)
    {
        DirectX::XMMATRIX matWorld = DirectX::XMMatrixScaling(axes[i].vScale.x, axes[i].vScale.y, axes[i].vScale.z) *
                                     DirectX::XMMatrixTranslation(axes[i].vOffset.x, axes[i].vOffset.y, axes[i].vOffset.z) *
                                     DirectX::XMMatrixTranslationFromVector(vPos);

        DirectX::XMMATRIX matWVP = matWorld * matView * matProj;

        SceneConstantBuffer cb = {};
        DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixTranspose(matWVP));
        
        if (bForPicking)
        {
            UINT nID = axes[i].nPickedID;
            cb.objectColorID.x = (nID & 0xFF) / 255.0f;
            cb.objectColorID.y = ((nID >> 8) & 0xFF) / 255.0f;
            cb.objectColorID.z = ((nID >> 16) & 0xFF) / 255.0f;
            cb.objectColorID.w = 1.0f;
            cb.meshColor = { 0, 0, 0, 0 };
            
            if (pRawData && pRawCB)
            {
                memcpy(pRawData + (nGizmoIdxStart + i) * 256, &cb, sizeof(cb));
                pCommandList->SetGraphicsRootConstantBufferView(0, pRawCB->GetGPUVirtualAddress() + (nGizmoIdxStart + i) * 256);
            }
        }
        else
        {
            UINT nID = axes[i].nPickedID;
            cb.objectColorID.x = (nID & 0xFF) / 255.0f;
            cb.objectColorID.y = ((nID >> 8) & 0xFF) / 255.0f;
            cb.objectColorID.z = ((nID >> 16) & 0xFF) / 255.0f;
            cb.objectColorID.w = 1.0f;
            cb.meshColor = axes[i].vColor;

            if (pCB)
            {
                pCB->Update(nGizmoIdxStart + i, &cb, sizeof(cb));
                pCommandList->SetGraphicsRootConstantBufferView(0, pCB->GetGPUVirtualAddress(nGizmoIdxStart + i));
            }
        }
        
        pCubeMesh->Render(pCommandList);
    }
}
