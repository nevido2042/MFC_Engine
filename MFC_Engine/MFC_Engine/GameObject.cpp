#include "pch.h"
#include "GameObject.h"
#include <algorithm>

CGameObject::CGameObject(const std::wstring& name)
    : m_name(name)
{
    m_pTransform = std::make_shared<CTransform>(this);
    m_components.push_back(m_pTransform);
}

CGameObject::~CGameObject()
{
}

std::shared_ptr<CGameObject> CGameObject::Create(const std::wstring& name)
{
    return std::make_shared<CGameObject>(name);
}

void CGameObject::AddChild(std::shared_ptr<CGameObject> child)
{
    if (child)
    {
        child->m_pParent = this;
        m_children.push_back(child);
    }
}

void CGameObject::RemoveChild(std::shared_ptr<CGameObject> child)
{
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end())
    {
        child->m_pParent = nullptr;
        m_children.erase(it);
    }
}
