#include "pch.h"
#include "Scene.h"
#include <algorithm>
#include <functional>
#include "MeshFilter.h"
#include "MeshRenderer.h"

CScene::CScene()
{
    // 테스트용 초기 오브젝트 생성
    auto mainCamera = CGameObject::Create(L"Main Camera");
    mainCamera->GetTransform()->m_position = { 0, 0, -10 };
    AddGameObject(mainCamera);
}

CScene::~CScene()
{
}

void CScene::RemoveGameObject(std::shared_ptr<CGameObject> obj)
{
    if (!obj) return;

    // 최상위 목록에서 제거 시도
    auto it = std::find(m_gameObjects.begin(), m_gameObjects.end(), obj);
    if (it != m_gameObjects.end())
    {
        m_gameObjects.erase(it);
        return;
    }

    // 부모가 있는 경우 부모로부터 제거
    auto pParent = obj->GetParent();
    if (pParent)
    {
        pParent->RemoveChild(obj);
    }
}

std::shared_ptr<CGameObject> CScene::FindGameObjectByID(UINT id)
{
    std::function<std::shared_ptr<CGameObject>(const std::vector<std::shared_ptr<CGameObject>>&)> search = 
        [&](const std::vector<std::shared_ptr<CGameObject>>& list) -> std::shared_ptr<CGameObject>
    {
        for (auto& obj : list)
        {
            if (obj->GetID() == id) return obj;
            auto found = search(obj->GetChildren());
            if (found) return found;
        }
        return nullptr;
    };
    return search(m_gameObjects);
}
