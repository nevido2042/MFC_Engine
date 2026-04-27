#pragma once
#include "Component.h"


class CTransform : public CComponent
{
public:
    CTransform(CGameObject* owner);

    DirectX::XMMATRIX GetWorldMatrix();

    DirectX::XMFLOAT3 m_vPosition;
    DirectX::XMFLOAT3 m_vRotation;
    DirectX::XMFLOAT3 m_vScale;
};
