#include "pch.h"
#include "Scene.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"

CScene::CScene()
{
    // 테스트용 초기 오브젝트 생성
    auto mainCamera = CGameObject::Create(L"Main Camera");
    mainCamera->GetTransform()->m_position = { 0, 0, -10 };
    AddGameObject(mainCamera);

    auto player = CGameObject::Create(L"Player Cube");
    player->AddComponent<CMeshFilter>()->m_meshName = L"Cube";
    player->AddComponent<CMeshRenderer>();
    AddGameObject(player);
}

CScene::~CScene()
{
}
