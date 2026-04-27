#include "pch.h"
#include "PickingSystem.h"
#include "PickingRenderTarget.h"
#include "GameObject.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "MeshFilter.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Camera.h"
#include "PrimitiveGenerator.h"
#include "Mesh.h"
#include "EngineStructs.h"
#include "GraphicsEngine.h"

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
    
    // 기본 래스터라이저 설정 (수동 초기화)
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

    // 기본 블렌드 설정 (수동 초기화)
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

    // 깊이 스텐실 설정 (수동 초기화)
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
    {
        m_pPickingRenderTarget->Resize(device.Get(), width, height);
    }
}

UINT CPickingSystem::Pick(int x, int y, CGraphicsEngine* pEngine, PickingOverlayFunc overlayFunc)
{
    if (!pEngine || !m_pPickingRenderTarget) return 0;

    auto commandQueue = pEngine->GetCommandQueue();
    auto commandAllocator = pEngine->GetCommandAllocator();
    auto commandList = pEngine->GetCommandList();
    auto rootSignature = pEngine->GetRootSignature();
    auto constantBuffer = pEngine->GetConstantBufferResource();
    auto cbvDataBegin = pEngine->GetConstantBufferPtr();
    int width = pEngine->GetWidth();
    int height = pEngine->GetHeight();

    // 커맨드 리스트 초기화
    commandAllocator->Reset();
    commandList->Reset(commandAllocator, nullptr);

    // 1. 렌더링 패스 수행
    RenderPickingPass(CSceneManager::GetInstance().GetActiveScene(), commandList, rootSignature, width, height, constantBuffer, cbvDataBegin, overlayFunc);

    // 2. 결과 읽기 예약
    m_pPickingRenderTarget->ReadPixelAsync(commandList, x, y);

    // 3. 커맨드 리스트 닫기 및 실행
    ThrowIfFailed(commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // 4. GPU 대기 (피킹 데이터 동기화)
    pEngine->WaitGPU();

    // 5. 결과 반환
    return m_pPickingRenderTarget->GetPickedID();
}

void CPickingSystem::RenderPickingPass(std::shared_ptr<CScene> pScene, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature, int width, int height, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin, PickingOverlayFunc overlayFunc)
{
    if (!pScene || !m_pPickingRenderTarget || !m_pickingPSO) return;

    // 루트 시그니처 및 PSO 설정
    commandList->SetGraphicsRootSignature(rootSignature);
    commandList->SetPipelineState(m_pickingPSO.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 렌더 타겟 설정
    auto rtv = m_pPickingRenderTarget->GetRTV();
    auto dsv = m_pPickingRenderTarget->GetDSV();
    commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    // 클리어
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 뷰포트 및 시저 설정
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, width, height };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    // 씬 오브젝트 렌더링
    int objIndex = 1;
    for (auto& pObj : pScene->GetGameObjects())
    {
        RenderGameObjectForPicking(pObj, objIndex, commandList, width, height, constantBuffer, cbvDataBegin);
    }

    // 추가 오버레이(기즈모 등) 렌더링 콜백 실행
    if (overlayFunc)
    {
        // 오버레이가 도형보다 우선적으로 선택되도록 깊이 버퍼를 클리어하여 덧그림
        commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        overlayFunc(commandList, constantBuffer, cbvDataBegin);
    }
}

void CPickingSystem::RenderGameObjectForPicking(std::shared_ptr<CGameObject> pObj, int& cbIndex, ID3D12GraphicsCommandList* commandList, int width, int height, ID3D12Resource* constantBuffer, UINT8* cbvDataBegin)
{
    if (!pObj) return;

    auto pTransform = pObj->GetTransform();
    auto pFilter = pObj->GetComponent<CMeshFilter>();
    auto pRenderer = pObj->GetComponent<CMeshRenderer>();

    if (pTransform && pFilter && pRenderer && pRenderer->m_bIsEnabled)
    {
        auto pMesh = CPrimitiveGenerator::GetInstance().GetPrimitiveMesh(pFilter->m_strMeshName);
        if (pMesh)
        {
            DirectX::XMMATRIX matView;
            {
                std::lock_guard<std::mutex> camLock(CSceneManager::GetInstance().GetCameraMutex());
                matView = CSceneManager::GetInstance().GetEditorCamera().GetViewMatrix();
            }

            float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
            DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 1000.0f);
            DirectX::XMMATRIX matWVP = pTransform->GetWorldMatrix() * matView * matProj;

            SceneConstantBuffer cb = {};
            DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixTranspose(matWVP));
            
            // 객체의 실제 고유 ID를 피킹 컬러로 사용 (24비트 RGB만 사용, A는 1.0 고정)
            UINT nID = pObj->GetID();
            cb.objectColorID.x = (nID & 0xFF) / 255.0f;
            cb.objectColorID.y = ((nID >> 8) & 0xFF) / 255.0f;
            cb.objectColorID.z = ((nID >> 16) & 0xFF) / 255.0f;
            cb.objectColorID.w = 1.0f;

            // 상수 버퍼 인덱스는 별도로 관리 (0~1023 제한 내에서 안전하게 사용)
            memcpy(cbvDataBegin + cbIndex * 256, &cb, sizeof(cb));
            commandList->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress() + cbIndex * 256);

            pMesh->Render(commandList);
            cbIndex++;
        }
    }

    for (auto& pChild : pObj->GetChildren())
    {
        RenderGameObjectForPicking(pChild, cbIndex, commandList, width, height, constantBuffer, cbvDataBegin);
    }
}

