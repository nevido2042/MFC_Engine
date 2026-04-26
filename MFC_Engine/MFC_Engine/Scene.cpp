#include "pch.h"
#include "Scene.h"

Scene::Scene()
{
    // 테스트용 초기 오브젝트 생성
    auto mainCamera = GameObject::Create(L"Main Camera");
    mainCamera->GetTransform()->m_position = { 0, 0, -10 };
    AddGameObject(mainCamera);

    auto directionalLight = GameObject::Create(L"Directional Light");
    AddGameObject(directionalLight);

    auto player = GameObject::Create(L"Player");
    
    auto childObj = GameObject::Create(L"Weapon");
    player->AddChild(childObj);
    
    AddGameObject(player);
}

Scene::~Scene()
{
}
