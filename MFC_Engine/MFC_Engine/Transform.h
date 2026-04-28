#pragma once
#include "Component.h"


class CTransform : public CComponent
{
public:
    CTransform(CGameObject* owner);

    DirectX::XMMATRIX GetWorldMatrix();

    virtual std::string GetComponentName() const override { return "CTransform"; }
    virtual void Serialize(nlohmann::json& j) const override;
    virtual void Deserialize(const nlohmann::json& j) override;

    DirectX::XMFLOAT3 m_vPosition;
    DirectX::XMFLOAT3 m_vRotation;
    DirectX::XMFLOAT3 m_vScale;
};
