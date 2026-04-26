#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Transform.h"

class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
    GameObject(const std::wstring& name);
    ~GameObject();

    static std::shared_ptr<GameObject> Create(const std::wstring& name);

    const std::wstring& GetName() const { return m_name; }
    void SetName(const std::wstring& name) { m_name = name; }

    std::shared_ptr<Transform> GetTransform() { return m_pTransform; }

    // Hierarchy management
    void AddChild(std::shared_ptr<GameObject> child);
    const std::vector<std::shared_ptr<GameObject>>& GetChildren() const { return m_children; }
    
    GameObject* GetParent() { return m_pParent; }

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

private:
    std::wstring m_name;
    GameObject* m_pParent = nullptr;
    std::shared_ptr<Transform> m_pTransform;
    std::vector<std::shared_ptr<Component>> m_components;
    std::vector<std::shared_ptr<GameObject>> m_children;
};
