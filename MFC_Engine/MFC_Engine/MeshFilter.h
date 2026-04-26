#pragma once
#include "Component.h"
#include <string>

/**
 * @class CMeshFilter
 * @brief 게임 오브젝트가 사용할 메쉬 데이터를 지정하는 컴포넌트입니다.
 */
class CMeshFilter : public CComponent
{
public:
    CMeshFilter(CGameObject* owner) : CComponent(owner) {}
    virtual ~CMeshFilter() {}

    std::wstring m_meshName = L"Cube"; // 메쉬 식별자 (기본값: Cube)
};
