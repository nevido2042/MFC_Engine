#include "pch.h"
#include "LightingPass.h"
#include "GBuffer.h"
#include "SwapChain.h"
#include "Scene.h"
#include "GameObject.h"
#include "ConstantBuffer.h"
#include "Light.h"
#include "EngineStructs.h"
#include <d3dcompiler.h>

void CLightingPass::Initialize(ID3D12Device* pDevice)
{
    // 1. Root Signature
    D3D12_DESCRIPTOR_RANGE srvTable;
    srvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvTable.NumDescriptors = 3;
    srvTable.BaseShaderRegister = 0;
    srvTable.RegisterSpace = 0;
    srvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameters[2];
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].Descriptor.RegisterSpace = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &srvTable;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 2;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &samplerDesc;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

    // 2. Pipeline State
    ComPtr<ID3DBlob> vs, ps;
    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    if (FAILED(D3DCompileFromFile(L"Lighting.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vs, &error))) {
        if (error) OutputDebugStringA((char*)error->GetBufferPointer());
    }
    if (FAILED(D3DCompileFromFile(L"Lighting.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &ps, &error))) {
        if (error) OutputDebugStringA((char*)error->GetBufferPointer());
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
    psoDesc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void CLightingPass::Execute(const RenderContext& context)
{
    context.pGBuffer->TransitionToShaderResource(context.pCommandList);
    
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = context.pMainSwapChain->GetRtvHandle();
    context.pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    
    const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    context.pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    context.pCommandList->SetPipelineState(m_pipelineState.Get());
    context.pCommandList->SetGraphicsRootSignature(m_rootSignature.Get());
    context.pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Setup Lighting constants
    SceneConstantBuffer cb = {};
    std::shared_ptr<CLight> pMainLight = nullptr;
    if (context.pScene)
    {
        for (auto& pObj : context.pScene->GetGameObjects())
        {
            auto pLight = pObj->GetComponent<CLight>();
            if (pLight)
            {
                pMainLight = pLight;
                break;
            }
        }
    }

    if (pMainLight)
    {
        DirectX::XMMATRIX lightWorld = pMainLight->GetOwner()->GetTransform()->GetWorldMatrix();
        DirectX::XMVECTOR lightForward = DirectX::XMVector3Normalize(lightWorld.r[2]);
        DirectX::XMStoreFloat4(&cb.lightDir, lightForward);
        cb.lightColor = pMainLight->m_vLightColor;
        cb.ambientColor = pMainLight->m_vAmbientColor;
    }
    else
    {
        cb.lightDir = { 0.0f, -1.0f, 1.0f, 0.0f };
        cb.lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        cb.ambientColor = { 0.2f, 0.2f, 0.2f, 1.0f };
    }
    
    context.pCB->Update(1023, &cb, sizeof(cb));
    context.pCommandList->SetGraphicsRootConstantBufferView(0, context.pCB->GetGPUVirtualAddress(1023));

    ID3D12DescriptorHeap* descriptorHeaps[] = { context.pGBuffer->GetSrvHeap() };
    context.pCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    context.pCommandList->SetGraphicsRootDescriptorTable(1, context.pGBuffer->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());

    context.pCommandList->DrawInstanced(3, 1, 0, 0);
}
