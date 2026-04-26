#pragma once
#include <vector>
#include <memory>
#include "GameObject.h"

class CScene
{
public:
    CScene();
    ~CScene();

    void AddGameObject(std::shared_ptr<CGameObject> obj) { m_gameObjects.push_back(obj); }
    const std::vector<std::shared_ptr<CGameObject>>& GetGameObjects() const { return m_gameObjects; }

private:
    std::vector<std::shared_ptr<CGameObject>> m_gameObjects;
};
