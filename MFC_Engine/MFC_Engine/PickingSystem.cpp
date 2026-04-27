#include "pch.h"
#include "PickingSystem.h"
#include "GraphicsEngine.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "MeshFilter.h"
#include "Transform.h"
#include "PrimitiveGenerator.h"


void CPickingSystem::Initialize(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature> rootSignature, int width, int height)
{
    // PSO 생성
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ThrowIfFailed(D3DCompileFromFile(L"Picking.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    ThrowIfFailed(D3DCompileFromFile(L"Picking.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.RasterizerState.MultisampleEnable = FALSE;
    psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    psoDesc.RasterizerState.ForcedSampleCount = 0;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

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

    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pickingPSO)));

    // CPicking 초기화
    m_pPickingRenderTarget = std::make_unique<CPickingRenderTarget>();
    m_pPickingRenderTarget->Initialize(device.Get(), width, height);
}

void CPickingSystem::Resize(ComPtr<ID3D12Device> device, int width, int height)
{
    if (m_pPickingRenderTarget)
        m_pPickingRenderTarget->Resize(device.Get(), width, height);
}

UINT CPickingSystem::Pick(int x, int y, ID3D12Device* device, ID3D12CommandQueue* queue, ID3D12CommandAllocator* allocator, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin, int width, int height)
{
    if (!m_pPickingRenderTarget) return 0;

    // 명령 리스트 리셋
    ThrowIfFailed(allocator->Reset());
    ThrowIfFailed(commandList->Reset(allocator, m_pickingPSO.Get()));
    commandList->SetGraphicsRootSignature(rootSignature);

    // 피킹 패스 렌더링
    RenderPickingPass(CSceneManager::GetInstance().GetActiveScene(), commandList, width, height, constantBuffer, cbvDataBegin);
    
    // 픽셀 읽기
    m_pPickingRenderTarget->ReadPixelAsync(commandList, x, y);

    ThrowIfFailed(commandList->Close());

    ID3D12CommandList* ppCommandLists[] = { commandList };
    queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    return 0; // 이제 결과는 GetPickedID()로 별도 호출
}

UINT CPickingSystem::GetPickedID()
{
    if (m_pPickingRenderTarget)
        return m_pPickingRenderTarget->GetPickedID();
    return 0;
}

void CPickingSystem::RenderPickingPass(std::shared_ptr<CScene> pScene, ID3D12GraphicsCommandList* commandList, int width, int height, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin)
{
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, width, height };

    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    auto rtv = m_pPickingRenderTarget->GetRTV();
    auto dsv = m_pPickingRenderTarget->GetDSV();

    commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
    
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // RootSignature는 외부에서 제공 (GraphicsEngine과 공유)
    // commandList->SetGraphicsRootSignature(rootSignature); // 이미 리셋 시 설정됨? 아님 명시적으로 해야함
    // GraphicsEngine::Pick에서 리셋 시 PSO는 설정하지만 RootSignature는 안할 수 있음
    
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (pScene)
    {
        int objIndex = 0;
        for (auto& pObj : pScene->GetGameObjects())
        {
            RenderGameObjectForPicking(pObj, objIndex, commandList, width, height, constantBuffer, cbvDataBegin);
        }
    }
}

void CPickingSystem::RenderGameObjectForPicking(std::shared_ptr<CGameObject> pObj, int& objIndex, ID3D12GraphicsCommandList* commandList, int width, int height, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin)
{
    if (objIndex >= 1024) return;

    auto pRenderer = pObj->GetComponent<CMeshRenderer>();
    if (pRenderer && pRenderer->m_isEnabled)
    {
        auto pFilter = pObj->GetComponent<CMeshFilter>();
        if (pFilter)
        {
            auto pTransform = pObj->GetTransform();
            if (pTransform)
            {
                DirectX::XMMATRIX matWorld = pTransform->GetWorldMatrix();
                DirectX::XMMATRIX matView;
                {
                    std::lock_guard<std::mutex> camLock(CSceneManager::GetInstance().GetCameraMutex());
                    matView = CSceneManager::GetInstance().GetEditorCamera().GetViewMatrix();
                }
                float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
                DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 100.0f);

                DirectX::XMMATRIX matWVP = matWorld * matView * matProj;

                SceneConstantBuffer cb;
                DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixTranspose(matWVP));
                
                UINT id = pObj->GetID();
                cb.objectColorID.x = (id & 0xFF) / 255.0f;
                cb.objectColorID.y = ((id >> 8) & 0xFF) / 255.0f;
                cb.objectColorID.z = ((id >> 16) & 0xFF) / 255.0f;
                cb.objectColorID.w = ((id >> 24) & 0xFF) / 255.0f;

                memcpy(cbvDataBegin + (objIndex * 256), &cb, sizeof(cb));

                commandList->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress() + (objIndex * 256));

                auto pMesh = CPrimitiveGenerator::GetInstance().GetPrimitiveMesh(pFilter->m_meshName);
                if (pMesh)
                {
                    pMesh->Render(commandList);
                }

                objIndex++;
            }
        }
    }

    for (auto& pChild : pObj->GetChildren())
    {
        RenderGameObjectForPicking(pChild, objIndex, commandList, width, height, constantBuffer, cbvDataBegin);
    }
}
