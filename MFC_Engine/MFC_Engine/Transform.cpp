#include "pch.h"
#include "Transform.h"
#include "GameObject.h"

CTransform::CTransform(CGameObject* owner)
    : CComponent(owner)
{
    m_position = { 0, 0, 0 };
    m_rotation = { 0, 0, 0 };
    m_scale = { 1, 1, 1 };
}

DirectX::XMMATRIX CTransform::GetWorldMatrix()
{
    DirectX::XMMATRIX matScale = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    DirectX::XMMATRIX matRot = DirectX::XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
    DirectX::XMMATRIX matTrans = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

    DirectX::XMMATRIX matLocal = matScale * matRot * matTrans;

    if (m_pOwner && m_pOwner->GetParent())
    {
        return matLocal * m_pOwner->GetParent()->GetTransform()->GetWorldMatrix();
    }

    return matLocal;
}
