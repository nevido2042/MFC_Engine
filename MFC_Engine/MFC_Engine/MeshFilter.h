#pragma once
#include "Component.h"
#include <string>

/**
 * @class CMeshFilter
 * @brief 게임 오브젝트가 사용할 메쉬 데이터를 지정하는 컴포넌트입니다.
 */
#pragma once
#include "Component.h"
#include <string>
#include "StringUtil.h"

/**
 * @class CMeshFilter
 * @brief 게임 오브젝트가 사용할 메쉬 데이터를 지정하는 컴포넌트입니다.
 */
class CMeshFilter : public CComponent
{
public:
    CMeshFilter(CGameObject* owner) : CComponent(owner) {}
    virtual ~CMeshFilter() {}

    virtual std::string GetComponentName() const override { return "CMeshFilter"; }
    
    virtual void Serialize(nlohmann::json& j) const override
    {
        j["MeshName"] = CStringUtil::WStringToUTF8(m_strMeshName);
    }

    virtual void Deserialize(const nlohmann::json& j) override
    {
        if (j.contains("MeshName"))
        {
            std::string meshName = j["MeshName"];
            m_strMeshName = CStringUtil::UTF8ToWString(meshName);
        }
    }

    std::wstring m_strMeshName = L"Cube"; // 메쉬 식별자 (기본값: Cube)
};
