#pragma once

#include <algorithm>
#include "GameObject.h"

class CScene
{
public:
    CScene();
    ~CScene();

    void SetName(const std::wstring& name) { m_strName = name; }
    const std::wstring& GetName() const { return m_strName; }

    void AddGameObject(std::shared_ptr<CGameObject> obj) { m_gameObjects.push_back(obj); }
    void RemoveGameObject(std::shared_ptr<CGameObject> obj);
    const std::vector<std::shared_ptr<CGameObject>>& GetGameObjects() const { return m_gameObjects; }

    std::shared_ptr<CGameObject> FindGameObjectByID(UINT id);

private:
    std::wstring m_strName;
    std::vector<std::shared_ptr<CGameObject>> m_gameObjects;
};
