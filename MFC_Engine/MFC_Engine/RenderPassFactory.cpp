#include "pch.h"
#include "RenderPassFactory.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "DebugPass.h"

CDX12RenderPassFactory::CDX12RenderPassFactory(ID3D12Device* pDevice)
    : m_pDevice(pDevice)
{
}

std::unique_ptr<CRenderPass> CDX12RenderPassFactory::CreatePass(const std::string& passName)
{
    std::unique_ptr<CRenderPass> pPass = nullptr;

    if (passName == "Geometry")
    {
        pPass = std::make_unique<CGeometryPass>();
    }
    else if (passName == "Lighting")
    {
        pPass = std::make_unique<CLightingPass>();
    }
    else if (passName == "Debug")
    {
        pPass = std::make_unique<CDebugPass>();
    }

    if (pPass)
    {
        pPass->Initialize(m_pDevice);
    }

    return pPass;
}
