#include "pch.h"
#include "GameObject.h"
#include <algorithm>
#include "StringUtil.h"
#include "Light.h"

UINT CGameObject::s_nNextID = 1;

CGameObject::CGameObject(const std::wstring &name)
    : m_strName(name), m_nID(s_nNextID++) {
  m_pTransform = std::make_shared<CTransform>(this);
  m_vecComponents.push_back(m_pTransform);
}

CGameObject::~CGameObject() {
  for (auto& child : m_vecChildren) {
    if (child) {
      child->m_pParent = nullptr;
    }
  }
}

std::shared_ptr<CGameObject> CGameObject::Create(const std::wstring &name) {
  return std::make_shared<CGameObject>(name);
}

void CGameObject::AddChild(std::shared_ptr<CGameObject> child) {
  if (child) {
    child->m_pParent = this;
    m_vecChildren.push_back(child);
  }
}

void CGameObject::RemoveChild(std::shared_ptr<CGameObject> child) {
  auto it = std::find(m_vecChildren.begin(), m_vecChildren.end(), child);
  if (it != m_vecChildren.end()) {
    child->m_pParent = nullptr;
    m_vecChildren.erase(it);
  }
}

void CGameObject::Serialize(nlohmann::json& j) const
{
    j["ID"] = m_nID;
    j["Name"] = CStringUtil::WStringToUTF8(m_strName);

    nlohmann::json componentsJson = nlohmann::json::array();
    for (const auto& comp : m_vecComponents)
    {
        nlohmann::json compJson;
        compJson["Type"] = comp->GetComponentName();
        nlohmann::json compData;
        comp->Serialize(compData);
        compJson["Data"] = compData;
        componentsJson.push_back(compJson);
    }
    j["Components"] = componentsJson;

    nlohmann::json childrenJson = nlohmann::json::array();
    for (const auto& child : m_vecChildren)
    {
        nlohmann::json childJson;
        child->Serialize(childJson);
        childrenJson.push_back(childJson);
    }
    j["Children"] = childrenJson;
}

#include "MeshFilter.h"
#include "MeshRenderer.h"
#include "ConstantBuffer.h"
#include "EngineStructs.h"
#include "SceneManager.h"
#include "Camera.h"
#include "PrimitiveGenerator.h"
#include "Mesh.h"

void CGameObject::Deserialize(const nlohmann::json& j)
{
    if (j.contains("ID"))
    {
        m_nID = j["ID"];
        if (m_nID >= s_nNextID)
        {
            s_nNextID = m_nID + 1;
        }
    }

    if (j.contains("Name"))
    {
        std::string nameStr = j["Name"];
        m_strName = CStringUtil::UTF8ToWString(nameStr);
    }

    // Transform is automatically created in constructor, so we just deserialize it if present
    // First, let's clear other components (if we are reusing this object, though usually it's freshly created)
    m_vecComponents.clear();
    m_pTransform = nullptr;

    if (j.contains("Components"))
    {
        for (const auto& compJson : j["Components"])
        {
            std::string type = compJson["Type"];
            std::shared_ptr<CComponent> newComp;

            if (type == "CTransform")
            {
                m_pTransform = std::make_shared<CTransform>(this);
                newComp = m_pTransform;
            }
            else if (type == "CMeshFilter")
            {
                newComp = std::make_shared<CMeshFilter>(this);
            }
            else if (type == "CMeshRenderer")
            {
                newComp = std::make_shared<CMeshRenderer>(this);
            }
            else if (type == "Light")
            {
                newComp = std::make_shared<CLight>(this);
            }

            if (newComp)
            {
                if (compJson.contains("Data"))
                {
                    newComp->Deserialize(compJson["Data"]);
                }
                m_vecComponents.push_back(newComp);
            }
        }
    }

    if (j.contains("Children"))
    {
        for (const auto& childJson : j["Children"])
        {
            auto childObj = CGameObject::Create(L"");
            childObj->Deserialize(childJson);
            AddChild(childObj);
        }
    }
}

void CGameObject::Render(ID3D12GraphicsCommandList* pCommandList, int& objIndex, CLight* pLight, CConstantBuffer* pCB, int nWidth, int nHeight)
{
    if (objIndex >= 1024) return;

    auto pRenderer = GetComponent<CMeshRenderer>();
    if (pRenderer && pRenderer->m_bIsEnabled)
    {
        auto pFilter = GetComponent<CMeshFilter>();
        if (pFilter)
        {
            auto pTransform = GetTransform();
            if (pTransform)
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
                cb.objectColorID = { 0.0f, 0.0f, 0.0f, 0.0f };
                cb.meshColor = { 0.0f, 0.0f, 0.0f, 0.0f }; // 기본적으로 무시

                if (pLight)
                {
                    // 빛의 방향은 Transform의 Rotation에 의해 결정되는 전방 벡터(Z축) 사용
                    DirectX::XMMATRIX lightWorld = pLight->GetOwner()->GetTransform()->GetWorldMatrix();
                    DirectX::XMVECTOR lightForward = DirectX::XMVector3Normalize(lightWorld.r[2]);
                    DirectX::XMStoreFloat4(&cb.lightDir, lightForward);
                    cb.lightColor = pLight->m_vLightColor;
                    cb.ambientColor = pLight->m_vAmbientColor;
                }
                else
                {
                    // 기본 빛
                    cb.lightDir = { 0.0f, -1.0f, 1.0f, 0.0f };
                    cb.lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
                    cb.ambientColor = { 0.2f, 0.2f, 0.2f, 1.0f };
                }

                if (pCB)
                {
                    pCB->Update(objIndex, &cb, sizeof(cb));
                    pCommandList->SetGraphicsRootConstantBufferView(0, pCB->GetGPUVirtualAddress(objIndex));
                }

                auto pMesh = CPrimitiveGenerator::GetInstance().GetPrimitiveMesh(pFilter->m_strMeshName);
                if (pMesh)
                {
                    pMesh->Render(pCommandList);
                }

                objIndex++;
            }
        }
    }

    // 자식들도 렌더링
    for (auto& pChild : GetChildren())
    {
        pChild->Render(pCommandList, objIndex, pLight, pCB, nWidth, nHeight);
    }
}
