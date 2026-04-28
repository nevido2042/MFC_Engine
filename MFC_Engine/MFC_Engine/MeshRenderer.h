#pragma once
#include "Component.h"

/**
 * @class CMeshRenderer
 * @brief MeshFilter의 메쉬를 실제로 화면에 그리는 역할을 하는 컴포넌트입니다.
 */
class CMeshRenderer : public CComponent
{
public:
    CMeshRenderer(CGameObject* owner) : CComponent(owner) {}
    virtual ~CMeshRenderer() {}

    virtual std::string GetComponentName() const override { return "CMeshRenderer"; }

    virtual void Serialize(nlohmann::json& j) const override
    {
        j["IsEnabled"] = m_bIsEnabled;
    }

    virtual void Deserialize(const nlohmann::json& j) override
    {
        if (j.contains("IsEnabled"))
        {
            m_bIsEnabled = j["IsEnabled"];
        }
    }

    // 렌더링 활성화 여부
    bool m_bIsEnabled = true;
};
