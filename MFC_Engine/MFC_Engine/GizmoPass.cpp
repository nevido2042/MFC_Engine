#include "pch.h"
#include "GizmoPass.h"
#include "Gizmo.h"
#include "GBuffer.h"
#include "SwapChain.h"
#include "ConstantBuffer.h"
#include "d3dx12.h"

void CGizmoPass::Initialize(ID3D12Device* pDevice)
{
    // 1. Root Signature (Standard D3D12 structs for stability)
    D3D12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].Descriptor.RegisterSpace = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = _countof(rootParameters);
    rootSigDesc.pParameters = rootParameters;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

    // 2. Pipeline State
    ComPtr<ID3DBlob> vs, ps;
    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    if (FAILED(D3DCompileFromFile(L"Gizmo.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", compileFlags, 0, &vs, &error)))
    {
        if (error) OutputDebugStringA((char*)error->GetBufferPointer());
    }
    if (FAILED(D3DCompileFromFile(L"Gizmo.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", compileFlags, 0, &ps, &error)))
    {
        if (error) OutputDebugStringA((char*)error->GetBufferPointer());
    }

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
    psoDesc.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
    
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;

    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    
    // Proper Blend State initialization
    D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
    rtBlendDesc.BlendEnable = FALSE;
    rtBlendDesc.LogicOpEnable = FALSE;
    rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlend = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        psoDesc.BlendState.RenderTarget[i] = rtBlendDesc;
    
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 4;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void CGizmoPass::Execute(const RenderContext& context)
{
    if (!context.resources.pGizmo || !context.scene.pSelectedObj) return;

    context.pCommandList->SetPipelineState(m_pipelineState.Get());
    context.pCommandList->SetGraphicsRootSignature(m_rootSignature.Get());
    
    // Transition GBuffer to RENDER_TARGET state
    context.resources.pGBuffer->TransitionToRenderTarget(context.pCommandList);

    // Set Render Targets (Target 0: Main, Target 1-3: GBuffer)
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[4] = {};
    rtvHandles[0] = context.resources.pMainSwapChain->GetRtvHandle();
    rtvHandles[1] = context.resources.pGBuffer->GetRtvHandle(1);
    rtvHandles[2] = context.resources.pGBuffer->GetRtvHandle(2);
    rtvHandles[3] = context.resources.pGBuffer->GetRtvHandle(3);
    
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = context.resources.pGBuffer->GetDsvHandle();
    context.pCommandList->OMSetRenderTargets(4, rtvHandles, FALSE, &dsvHandle);

    context.pCommandList->RSSetViewports(1, &context.resources.pMainSwapChain->GetViewport());
    context.pCommandList->RSSetScissorRects(1, &context.resources.pMainSwapChain->GetScissorRect());

    context.resources.pGizmo->Render(context.pCommandList, m_rootSignature.Get(), context.scene.pSelectedObj, context.nWidth, context.nHeight, context.pCB);

    // Transition GBuffer back to SHADER_RESOURCE state for next frame/passes
    context.resources.pGBuffer->TransitionToShaderResource(context.pCommandList);
}
