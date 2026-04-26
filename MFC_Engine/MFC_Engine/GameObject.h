#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Transform.h"

class CGameObject : public std::enable_shared_from_this<CGameObject>
{
public:
    CGameObject(const std::wstring& name);
    ~CGameObject();

    static std::shared_ptr<CGameObject> Create(const std::wstring& name);

    const std::wstring& GetName() const { return m_name; }
    void SetName(const std::wstring& name) { m_name = name; }

    std::shared_ptr<CTransform> GetTransform() { return m_pTransform; }

    // Hierarchy management
    void AddChild(std::shared_ptr<CGameObject> child);
    const std::vector<std::shared_ptr<CGameObject>>& GetChildren() const { return m_children; }
    
    CGameObject* GetParent() { return m_pParent; }

    // Component management
    template<typename T>
    std::shared_ptr<T> GetComponent()
    {
        for (auto& component : m_components)
        {
            auto casted = std::dynamic_pointer_cast<T>(component);
            if (casted) return casted;
        }
        return nullptr;
    }

    template<typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args&&... args)
    {
        auto component = std::make_shared<T>(this, std::forward<Args>(args)...);
        m_components.push_back(component);
        return component;
    }

private:
    std::wstring m_name;
    CGameObject* m_pParent = nullptr;
    std::shared_ptr<CTransform> m_pTransform;
    std::vector<std::shared_ptr<CComponent>> m_components;
    std::vector<std::shared_ptr<CGameObject>> m_children;
};
