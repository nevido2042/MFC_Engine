#pragma once
#include <string>
#include "json.hpp"

class CGameObject;

class CComponent
{
public:
    CComponent(CGameObject* owner) : m_pOwner(owner) {}
    virtual ~CComponent() {}

    virtual void Update() {}
    
    // 직렬화 인터페이스
    virtual std::string GetComponentName() const = 0;
    virtual void Serialize(nlohmann::json& j) const {}
    virtual void Deserialize(const nlohmann::json& j) {}

    CGameObject* GetOwner() { return m_pOwner; }

protected:
    CGameObject* m_pOwner;
};
