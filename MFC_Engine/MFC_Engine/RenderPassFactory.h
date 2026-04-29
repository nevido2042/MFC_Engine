#pragma once
#include <memory>
#include <string>

class CRenderPass;
struct ID3D12Device;

/**
 * @interface IRenderPassFactory
 * @brief 렌더 패스 생성을 위한 추상 인터페이스입니다. (추상 팩토리)
 */
class IRenderPassFactory
{
public:
    virtual ~IRenderPassFactory() {}
    virtual std::unique_ptr<CRenderPass> CreatePass(const std::string& passName) = 0;
};

/**
 * @class CDX12RenderPassFactory
 * @brief DirectX 12 전용 렌더 패스 생성 팩토리입니다.
 */
class CDX12RenderPassFactory : public IRenderPassFactory
{
public:
    CDX12RenderPassFactory(ID3D12Device* pDevice);
    virtual ~CDX12RenderPassFactory() {}

    std::unique_ptr<CRenderPass> CreatePass(const std::string& passName) override;

private:
    ID3D12Device* m_pDevice;
};
