#pragma once
#include "RenderPass.h"

class CGizmoPass : public CRenderPass
{
public:
    virtual void Initialize(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& context) override;
};
