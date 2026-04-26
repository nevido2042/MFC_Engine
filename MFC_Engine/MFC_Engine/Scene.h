#pragma once
#include <vector>
#include <memory>
#include "GameObject.h"

class Scene
{
public:
    Scene();
    ~Scene();

    void AddGameObject(std::shared_ptr<GameObject> obj) { m_gameObjects.push_back(obj); }
    const std::vector<std::shared_ptr<GameObject>>& GetGameObjects() const { return m_gameObjects; }

private:
    std::vector<std::shared_ptr<GameObject>> m_gameObjects;
};
