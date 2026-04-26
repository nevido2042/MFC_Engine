#pragma once
#include <string>

class GameObject;

class Component
{
public:
    Component(GameObject* owner) : m_pOwner(owner) {}
    virtual ~Component() {}

    virtual void Update() {}
    
    GameObject* GetOwner() { return m_pOwner; }

protected:
    GameObject* m_pOwner;
};
