#include "pch.h"
#include "Transform.h"
#include "GameObject.h"

CTransform::CTransform(CGameObject* owner)
    : CComponent(owner)
{
    m_vPosition = { 0, 0, 0 };
    m_vRotation = { 0, 0, 0 };
    m_vScale = { 1, 1, 1 };
}

DirectX::XMMATRIX CTransform::GetWorldMatrix()
{
    DirectX::XMMATRIX matScale = DirectX::XMMatrixScaling(m_vScale.x, m_vScale.y, m_vScale.z);
    DirectX::XMMATRIX matRot = DirectX::XMMatrixRotationRollPitchYaw(m_vRotation.x, m_vRotation.y, m_vRotation.z);
    DirectX::XMMATRIX matTrans = DirectX::XMMatrixTranslation(m_vPosition.x, m_vPosition.y, m_vPosition.z);

    DirectX::XMMATRIX matLocal = matScale * matRot * matTrans;

    if (m_pOwner && m_pOwner->GetParent())
    {
        return matLocal * m_pOwner->GetParent()->GetTransform()->GetWorldMatrix();
    }

    return matLocal;
}

void CTransform::Serialize(nlohmann::json& j) const
{
    j["Position"] = { m_vPosition.x, m_vPosition.y, m_vPosition.z };
    j["Rotation"] = { m_vRotation.x, m_vRotation.y, m_vRotation.z };
    j["Scale"] = { m_vScale.x, m_vScale.y, m_vScale.z };
}

void CTransform::Deserialize(const nlohmann::json& j)
{
    if (j.contains("Position"))
    {
        m_vPosition.x = j["Position"][0];
        m_vPosition.y = j["Position"][1];
        m_vPosition.z = j["Position"][2];
    }
    if (j.contains("Rotation"))
    {
        m_vRotation.x = j["Rotation"][0];
        m_vRotation.y = j["Rotation"][1];
        m_vRotation.z = j["Rotation"][2];
    }
    if (j.contains("Scale"))
    {
        m_vScale.x = j["Scale"][0];
        m_vScale.y = j["Scale"][1];
        m_vScale.z = j["Scale"][2];
    }
}
