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
