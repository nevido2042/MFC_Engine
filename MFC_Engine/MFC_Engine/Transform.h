#pragma once
#include "Component.h"
#include <directxmath.h>

class CTransform : public CComponent
{
public:
    CTransform(CGameObject* owner) : CComponent(owner) 
    {
        m_position = { 0, 0, 0 };
        m_rotation = { 0, 0, 0 };
        m_scale = { 1, 1, 1 };
    }

    DirectX::XMMATRIX GetWorldMatrix()
    {
        DirectX::XMMATRIX matScale = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
        DirectX::XMMATRIX matRot = DirectX::XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
        DirectX::XMMATRIX matTrans = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
        return matScale * matRot * matTrans;
    }

    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;
    DirectX::XMFLOAT3 m_scale;
};
