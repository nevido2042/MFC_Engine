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

    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;
    DirectX::XMFLOAT3 m_scale;
};
