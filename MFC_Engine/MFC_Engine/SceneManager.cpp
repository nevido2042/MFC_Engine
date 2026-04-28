#include "pch.h"
#include "SceneManager.h"
#include "SceneSerializer.h"

bool CSceneManager::SaveScene(const std::wstring& filePath)
{
    if (!m_pActiveScene) return false;

    CSceneSerializer serializer(m_pActiveScene);
    return serializer.Serialize(filePath);
}

bool CSceneManager::LoadScene(const std::wstring& filePath)
{
    // 새 씬 생성
    auto newScene = std::make_shared<CScene>();
    
    // 기본 CScene 생성자에서 Main Camera를 자동 생성하므로 불러오기 전에 비워줍니다.
    // 하지만 CScene::CScene()에 하드코딩되어 있으므로 새로 만든 씬의 오브젝트를 전부 제거합니다.
    auto rootObjects = newScene->GetGameObjects();
    for (auto obj : rootObjects)
    {
        newScene->RemoveGameObject(obj);
    }

    CSceneSerializer serializer(newScene);
    if (serializer.Deserialize(filePath))
    {
        SetActiveScene(newScene);
        return true;
    }
    
    return false;
}
