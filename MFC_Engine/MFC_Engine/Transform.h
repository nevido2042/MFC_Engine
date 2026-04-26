#pragma once
#include "Component.h"
#include <directxmath.h>

class CTransform : public CComponent
{
public:
    CTransform(CGameObject* owner);

    DirectX::XMMATRIX GetWorldMatrix();

    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_rotation;
    DirectX::XMFLOAT3 m_scale;
};
