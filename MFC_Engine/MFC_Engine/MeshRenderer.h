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

    virtual std::string GetComponentName() const override { return "CMeshRenderer"; }

    virtual void Serialize(nlohmann::json& j) const override;
    virtual void Deserialize(const nlohmann::json& j) override;

    // 렌더링 함수 추가
    void Render(struct ID3D12GraphicsCommandList* pCommandList, int& objIndex, class CConstantBuffer* pCB, int nWidth, int nHeight);

    // 렌더링 활성화 여부
    bool m_bIsEnabled = true;
};
