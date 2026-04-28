#include "pch.h"
#include "Light.h"

CLight::CLight(CGameObject* pOwner)
    : CComponent(pOwner)
    , m_eLightType(ELightType::Directional)
    , m_vLightColor(1.0f, 1.0f, 1.0f, 1.0f)
    , m_vAmbientColor(0.2f, 0.2f, 0.2f, 1.0f)
{
}

void CLight::Serialize(nlohmann::json& j) const
{
    j["LightType"] = static_cast<int>(m_eLightType);
    j["LightColor"] = { m_vLightColor.x, m_vLightColor.y, m_vLightColor.z, m_vLightColor.w };
    j["AmbientColor"] = { m_vAmbientColor.x, m_vAmbientColor.y, m_vAmbientColor.z, m_vAmbientColor.w };
}

void CLight::Deserialize(const nlohmann::json& j)
{
    if (j.contains("LightType"))
        m_eLightType = static_cast<ELightType>(j["LightType"].get<int>());
    
    if (j.contains("LightColor"))
    {
        m_vLightColor.x = j["LightColor"][0];
        m_vLightColor.y = j["LightColor"][1];
        m_vLightColor.z = j["LightColor"][2];
        m_vLightColor.w = j["LightColor"][3];
    }

    if (j.contains("AmbientColor"))
    {
        m_vAmbientColor.x = j["AmbientColor"][0];
        m_vAmbientColor.y = j["AmbientColor"][1];
        m_vAmbientColor.z = j["AmbientColor"][2];
        m_vAmbientColor.w = j["AmbientColor"][3];
    }
}
