#pragma once
#include "Component.h"
#include <DirectXMath.h>

enum class ELightType
{
    Directional,
    Point
};

class CLight : public CComponent
{
public:
    CLight(CGameObject* pOwner);
    virtual ~CLight() = default;

    ELightType m_eLightType;
    DirectX::XMFLOAT4 m_vLightColor;
    DirectX::XMFLOAT4 m_vAmbientColor;

    virtual std::string GetComponentName() const override { return "Light"; }
    virtual void Serialize(nlohmann::json& j) const override;
    virtual void Deserialize(const nlohmann::json& j) override;
};
