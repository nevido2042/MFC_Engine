#pragma once
#include <string>
#include <memory>
#include "Scene.h"

class CSceneSerializer
{
public:
    CSceneSerializer(std::shared_ptr<CScene> scene);

    bool Serialize(const std::wstring& filepath);
    bool Deserialize(const std::wstring& filepath);

private:
    std::shared_ptr<CScene> m_pScene;
};
