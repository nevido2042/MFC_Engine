#include "pch.h"
#include "Scene.h"

CScene::CScene()
{
    // 테스트용 초기 오브젝트 생성
    auto mainCamera = CGameObject::Create(L"Main Camera");
    mainCamera->GetTransform()->m_position = { 0, 0, -10 };
    AddGameObject(mainCamera);

    auto directionalLight = CGameObject::Create(L"Directional Light");
    AddGameObject(directionalLight);

    auto player = CGameObject::Create(L"Player");
    
    auto childObj = CGameObject::Create(L"Weapon");
    player->AddChild(childObj);
    
    AddGameObject(player);
}

CScene::~CScene()
{
}
