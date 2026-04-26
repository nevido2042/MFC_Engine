#pragma once
#include <string>

class CGameObject;

class CComponent
{
public:
    CComponent(CGameObject* owner) : m_pOwner(owner) {}
    virtual ~CComponent() {}

    virtual void Update() {}
    
    CGameObject* GetOwner() { return m_pOwner; }

protected:
    CGameObject* m_pOwner;
};
