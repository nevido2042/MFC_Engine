#pragma once
#include "Component.h"

/**
 * @class CMeshRenderer
 * @brief MeshFilter의 메쉬를 실제로 화면에 그리는 역할을 하는 컴포넌트입니다.
 */
class CMeshRenderer : public CComponent
{
public:
    CMeshRenderer(CGameObject* owner) : CComponent(owner) {}
    virtual ~CMeshRenderer() {}

    // 렌더링 활성화 여부
    bool m_isEnabled = true;
};
