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
