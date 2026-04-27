#include "pch.h"
#include "GameObject.h"
#include <algorithm>

UINT CGameObject::s_nNextID = 1;

CGameObject::CGameObject(const std::wstring &name)
    : m_strName(name), m_nID(s_nNextID++) {
  m_pTransform = std::make_shared<CTransform>(this);
  m_vecComponents.push_back(m_pTransform);
}

CGameObject::~CGameObject() {}

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
