#include "pch.h"
#include "GameObject.h"

GameObject::GameObject(const std::wstring& name)
    : m_name(name)
{
    // Transform은 모든 GameObject에 기본적으로 포함됩니다.
    m_pTransform = std::make_shared<Transform>(this);
    m_components.push_back(m_pTransform);
}

GameObject::~GameObject()
{
}

std::shared_ptr<GameObject> GameObject::Create(const std::wstring& name)
{
    return std::make_shared<GameObject>(name);
}

void GameObject::AddChild(std::shared_ptr<GameObject> child)
{
    if (child)
    {
        child->m_pParent = this;
        m_children.push_back(child);
    }
}
