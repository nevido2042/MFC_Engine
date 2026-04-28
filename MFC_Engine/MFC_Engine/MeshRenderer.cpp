#include "pch.h"
#include "MeshRenderer.h"
#include "GameObject.h"
#include "MeshFilter.h"
#include "ConstantBuffer.h"
#include "SceneManager.h"
#include "Camera.h"
#include "EngineStructs.h"
#include "PrimitiveGenerator.h"

void CMeshRenderer::Serialize(nlohmann::json& j) const
{
    j["IsEnabled"] = m_bIsEnabled;
}

void CMeshRenderer::Deserialize(const nlohmann::json& j)
{
    if (j.contains("IsEnabled"))
    {
        m_bIsEnabled = j["IsEnabled"];
    }
}

void CMeshRenderer::Render(ID3D12GraphicsCommandList* pCommandList, int& objIndex, CConstantBuffer* pCB, int nWidth, int nHeight)
{
    if (!m_bIsEnabled) return;

    CGameObject* pOwner = GetOwner();
    auto pFilter = pOwner->GetComponent<CMeshFilter>();
    auto pTransform = pOwner->GetTransform();

    if (pFilter && pTransform)
    {
        // 행렬 계산
        DirectX::XMMATRIX matWorld = pTransform->GetWorldMatrix();
        
        DirectX::XMMATRIX matView;
        {
            std::lock_guard<std::mutex> camLock(CSceneManager::GetInstance().GetCameraMutex());
            matView = CSceneManager::GetInstance().GetEditorCamera().GetViewMatrix();
        }

        float aspectRatio = static_cast<float>(nWidth) / static_cast<float>(nHeight);
        DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 1000.0f);

        DirectX::XMMATRIX matWVP = matWorld * matView * matProj;

        // 상수 버퍼 업데이트
        SceneConstantBuffer cb = {};
        DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixTranspose(matWVP));
        DirectX::XMStoreFloat4x4(&cb.matWorld, DirectX::XMMatrixTranspose(matWorld));
        // Encode Object ID for Picking
        UINT nID = pOwner->GetID();
        cb.objectColorID.x = (nID & 0xFF) / 255.0f;
        cb.objectColorID.y = ((nID >> 8) & 0xFF) / 255.0f;
        cb.objectColorID.z = ((nID >> 16) & 0xFF) / 255.0f;
        cb.objectColorID.w = 1.0f;
        cb.meshColor = { 0.0f, 0.0f, 0.0f, 0.0f }; 

        if (pCB)
        {
            pCB->Update(objIndex, &cb, sizeof(cb));
            pCommandList->SetGraphicsRootConstantBufferView(0, pCB->GetGPUVirtualAddress(objIndex));
        }

        // MeshFilter의 데이터를 참조하여 실제 렌더링 수행
        auto pMesh = CPrimitiveGenerator::GetInstance().GetPrimitiveMesh(pFilter->m_strMeshName);
        if (pMesh)
        {
            pMesh->Render(pCommandList);
        }
        objIndex++;
    }
}
