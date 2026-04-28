#include "pch.h"
#include "SceneSerializer.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include "SceneManager.h"
#include "StringUtil.h"

CSceneSerializer::CSceneSerializer(std::shared_ptr<CScene> scene)
    : m_pScene(scene)
{
}

bool CSceneSerializer::Serialize(const std::wstring& filepath)
{
    if (!m_pScene) return false;

    nlohmann::json j;
    j["SceneName"] = CStringUtil::WStringToUTF8(m_pScene->GetName());

    // 에디터 카메라 저장
    auto& camera = CSceneManager::GetInstance().GetEditorCamera();
    auto pos = camera.GetPosition();
    nlohmann::json camJson;
    camJson["Position"] = { pos.x, pos.y, pos.z };
    camJson["Pitch"] = camera.GetPitch();
    camJson["Yaw"] = camera.GetYaw();
    j["EditorCamera"] = camJson;


    nlohmann::json objectsJson = nlohmann::json::array();
    for (const auto& obj : m_pScene->GetGameObjects())
    {
        // 최상위 오브젝트들만 직렬화하면 자식들은 내부적으로 직렬화 됨
        nlohmann::json objJson;
        obj->Serialize(objJson);
        objectsJson.push_back(objJson);
    }
    j["GameObjects"] = objectsJson;

    std::ofstream file(filepath);
    if (file.is_open())
    {
        file << j.dump(4);
        file.close();
        return true;
    }

    return false;
}

bool CSceneSerializer::Deserialize(const std::wstring& filepath)
{
    if (!m_pScene) return false;

    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    nlohmann::json j;
    try
    {
        file >> j;
    }
    catch (nlohmann::json::parse_error&)
    {
        // Parse error handling
        return false;
    }

    if (j.contains("SceneName"))
    {
        std::string name = j["SceneName"];
        m_pScene->SetName(CStringUtil::UTF8ToWString(name));
    }

    // 에디터 카메라 로드
    if (j.contains("EditorCamera"))
    {
        auto& camJson = j["EditorCamera"];
        auto& camera = CSceneManager::GetInstance().GetEditorCamera();
        
        if (camJson.contains("Position"))
        {
            auto posArray = camJson["Position"];
            camera.SetPosition(DirectX::XMFLOAT3(posArray[0], posArray[1], posArray[2]));
        }
        if (camJson.contains("Pitch"))
        {
            camera.SetPitch(camJson["Pitch"]);
        }
        if (camJson.contains("Yaw"))
        {
            camera.SetYaw(camJson["Yaw"]);
        }
    }

    // 씬을 불러오기 전에 기존 오브젝트들을 지울 필요가 있음
    // SceneManager 측에서 처리하거나 여기서 지울 수 있음
    // 안전하게 SceneManager 측에서 Clear 처리를 하는 것이 좋음

    if (j.contains("GameObjects"))
    {
        for (const auto& objJson : j["GameObjects"])
        {
            auto obj = CGameObject::Create(L"");
            obj->Deserialize(objJson);
            m_pScene->AddGameObject(obj);
        }
    }

    return true;
}
