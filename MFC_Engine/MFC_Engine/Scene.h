#pragma once
#include <vector>
#include <memory>
#include "GameObject.h"

class CScene
{
public:
    CScene();
    ~CScene();

    void SetName(const std::wstring& name) { m_strName = name; }
    const std::wstring& GetName() const { return m_strName; }

    void AddGameObject(std::shared_ptr<CGameObject> obj) { m_gameObjects.push_back(obj); }
    const std::vector<std::shared_ptr<CGameObject>>& GetGameObjects() const { return m_gameObjects; }

private:
    std::wstring m_strName;
    std::vector<std::shared_ptr<CGameObject>> m_gameObjects;
};
