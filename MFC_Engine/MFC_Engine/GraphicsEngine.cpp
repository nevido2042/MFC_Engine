#include "pch.h"
#include "GraphicsEngine.h"
#include "Scene.h"
#include "Transform.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "Light.h"
#include "PickingSystem.h"
#include "SceneManager.h"
#include "Gizmo.h"

// 쉐이더 컴파일 라이브러리 링크

CGraphicsEngine::CGraphicsEngine()
    : m_bIsInitialized(false)
    , m_nWidth(0)
    , m_nHeight(0)
{
}

CGraphicsEngine::~CGraphicsEngine()
{
}

/**
 * @brief 엔진의 모든 DX12 초기 설정을 수행합니다.
 */
bool CGraphicsEngine::Initialize(HWND hWnd, int width, int height)
{
    m_nWidth = (width > 0) ? width : 1;
    m_nHeight = (height > 0) ? height : 1;

    m_timeManager.Initialize();

    m_pDevice = std::make_unique<CDevice>();
    m_pDevice->Initialize(hWnd, m_nWidth, m_nHeight);

    CreateRootSignature();
    CreatePipelineState();

    CPickingSystem::GetInstance().Initialize(m_pDevice->GetDevice(), m_rootSignatureDeferred, m_nWidth, m_nHeight);
    CPrimitiveGenerator::GetInstance().Initialize(m_pDevice->GetDevice());
    CreateConstantBuffer();

    m_bIsInitialized = true;
    return true;
}


/**
 * @brief 쉐이더 자원 바인딩을 위한 루트 시그니처를 생성합니다.
 */
void CGraphicsEngine::CreateRootSignature()
{
    // 1. Deferred Root Signature (Geometry Pass)
    D3D12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].Descriptor.RegisterSpace = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(m_pDevice->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatureDeferred)));

    // 2. Lighting Root Signature
    D3D12_DESCRIPTOR_RANGE srvTable;
    srvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvTable.NumDescriptors = 3;
    srvTable.BaseShaderRegister = 0;
    srvTable.RegisterSpace = 0;
    srvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER lightRootParameters[2];
    lightRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    lightRootParameters[0].Descriptor.ShaderRegister = 0;
    lightRootParameters[0].Descriptor.RegisterSpace = 0;
    lightRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    lightRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    lightRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    lightRootParameters[1].DescriptorTable.pDescriptorRanges = &srvTable;
    lightRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC lightRootSigDesc = {};
    lightRootSigDesc.NumParameters = 2;
    lightRootSigDesc.pParameters = lightRootParameters;
    lightRootSigDesc.NumStaticSamplers = 1;
    lightRootSigDesc.pStaticSamplers = &samplerDesc;
    lightRootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ThrowIfFailed(D3D12SerializeRootSignature(&lightRootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(m_pDevice->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatureLighting)));

    // 3. Debug Root Signature (No Constant Buffer needed)
    D3D12_ROOT_SIGNATURE_DESC debugRootSigDesc = {};
    debugRootSigDesc.NumParameters = 1;
    debugRootSigDesc.pParameters = &lightRootParameters[1];
    debugRootSigDesc.NumStaticSamplers = 1;
    debugRootSigDesc.pStaticSamplers = &samplerDesc;
    debugRootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ThrowIfFailed(D3D12SerializeRootSignature(&debugRootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(m_pDevice->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignatureDebug)));
}

/**
 * @brief 그래픽 파이프라인 상태 객체(PSO)를 생성합니다.
 */
void CGraphicsEngine::CreatePipelineState()
{
    ComPtr<ID3DBlob> deferredVS, deferredPS;
    ComPtr<ID3DBlob> lightingVS, lightingPS;
    ComPtr<ID3DBlob> debugVS, debugPS;
    ComPtr<ID3DBlob> error;

#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    // Deferred Pass Shaders
    if (FAILED(D3DCompileFromFile(L"Deferred.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &deferredVS, &error))) {
        OutputDebugStringA((char*)error->GetBufferPointer());
    }
    if (FAILED(D3DCompileFromFile(L"Deferred.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &deferredPS, &error))) {
        OutputDebugStringA((char*)error->GetBufferPointer());
    }

    // Lighting Pass Shaders
    if (FAILED(D3DCompileFromFile(L"Lighting.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &lightingVS, &error))) {
        OutputDebugStringA((char*)error->GetBufferPointer());
    }
    if (FAILED(D3DCompileFromFile(L"Lighting.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &lightingPS, &error))) {
        OutputDebugStringA((char*)error->GetBufferPointer());
    }

    // Debug Pass Shaders
    if (FAILED(D3DCompileFromFile(L"DebugGBuffer.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &debugVS, &error))) {
        OutputDebugStringA((char*)error->GetBufferPointer());
    }
    if (FAILED(D3DCompileFromFile(L"DebugGBuffer.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &debugPS, &error))) {
        OutputDebugStringA((char*)error->GetBufferPointer());
    }

    // 입력 레이아웃 정의 (Geometry Pass용)
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 1. Deferred Pipeline State (Geometry Pass)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignatureDeferred.Get();
    psoDesc.VS = { deferredVS->GetBufferPointer(), deferredVS->GetBufferSize() };
    psoDesc.PS = { deferredPS->GetBufferPointer(), deferredPS->GetBufferSize() };
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
    
    // G-Buffer Render Targets
    psoDesc.NumRenderTargets = 3;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT; // Position
    psoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT; // Normal
    psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;      // Albedo
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count = 1;

    ThrowIfFailed(m_pDevice->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateDeferred)));

    // --- 기즈모용 PSO ---
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.NumRenderTargets = 1; // 기즈모는 SwapChain에 그림
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
    psoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
    ThrowIfFailed(m_pDevice->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStateGizmo)));

    // 2. Lighting Pipeline State
    D3D12_GRAPHICS_PIPELINE_STATE_DESC lightingPsoDesc = psoDesc;
    lightingPsoDesc.InputLayout = { nullptr, 0 }; // Full-screen Quad generated in VS
    lightingPsoDesc.pRootSignature = m_rootSignatureLighting.Get();
    lightingPsoDesc.VS = { lightingVS->GetBufferPointer(), lightingVS->GetBufferSize() };
    lightingPsoDesc.PS = { lightingPS->GetBufferPointer(), lightingPS->GetBufferSize() };
    lightingPsoDesc.DepthStencilState.DepthEnable = FALSE; // No depth test for lighting pass quad
    lightingPsoDesc.NumRenderTargets = 1;
    lightingPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    lightingPsoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
    lightingPsoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
    
    ThrowIfFailed(m_pDevice->GetDevice()->CreateGraphicsPipelineState(&lightingPsoDesc, IID_PPV_ARGS(&m_pipelineStateLighting)));

    // 3. Debug G-Buffer Pipeline State
    D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc = lightingPsoDesc;
    debugPsoDesc.pRootSignature = m_rootSignatureDebug.Get();
    debugPsoDesc.VS = { debugVS->GetBufferPointer(), debugVS->GetBufferSize() };
    debugPsoDesc.PS = { debugPS->GetBufferPointer(), debugPS->GetBufferSize() };
    
    ThrowIfFailed(m_pDevice->GetDevice()->CreateGraphicsPipelineState(&debugPsoDesc, IID_PPV_ARGS(&m_pipelineStateDebug)));
}



/**
 * @brief 쉐이더 상수를 담을 버퍼를 생성하고 메모리 매핑을 수행합니다.
 */
void CGraphicsEngine::CreateConstantBuffer()
{
    m_pConstantBuffer = std::make_unique<CConstantBuffer>();
    m_pConstantBuffer->Initialize(m_pDevice->GetDevice(), sizeof(SceneConstantBuffer), 1024);
}


void CGraphicsEngine::Render(std::shared_ptr<CScene> pScene, std::shared_ptr<CGameObject> pSelectedObj, CGizmo* pGizmo)
{
    if (!m_bIsInitialized) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_timeManager.Update();

    m_pDevice->PrepareRender(); // Clears main DSV, handles SwapChain transitions
    auto commandList = m_pDevice->GetCommandList();

    // ==========================================
    // Pass 1: Geometry Pass (G-Buffer)
    // ==========================================
    m_pDevice->TransitionGBuffersToRenderTarget();
    m_pDevice->ClearAndSetGBuffers();

    commandList->SetPipelineState(m_pipelineStateDeferred.Get());
    commandList->SetGraphicsRootSignature(m_rootSignatureDeferred.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    std::shared_ptr<CLight> pMainLight = nullptr;

    if (pScene)
    {
        const auto& gameObjects = pScene->GetGameObjects();
        
        for (auto& pObj : gameObjects)
        {
            auto pLight = pObj->GetComponent<CLight>();
            if (pLight)
            {
                pMainLight = pLight;
                break;
            }
        }

        int objIndex = 0;
        for (auto& pObj : gameObjects)
        {
            RenderGameObject(pObj, objIndex, pMainLight.get());
        }
    }
    else
    {
        // 씬이 없을 경우 기본 회전 큐브 하나만 그림 (디버그용)
        float totalTime = m_timeManager.GetTotalTime();
        DirectX::XMMATRIX matWorld = DirectX::XMMatrixRotationRollPitchYaw(totalTime * 0.5f, totalTime, 0.0f);
        
        DirectX::XMMATRIX matView;
        {
            std::lock_guard<std::mutex> camLock(CSceneManager::GetInstance().GetCameraMutex());
            matView = CSceneManager::GetInstance().GetEditorCamera().GetViewMatrix();
        }

        float aspectRatio = static_cast<float>(m_nWidth) / static_cast<float>(m_nHeight);
        DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 100.0f);
        DirectX::XMMATRIX matWVP = matWorld * matView * matProj;

        SceneConstantBuffer cb = {};
        DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixTranspose(matWVP));
        DirectX::XMStoreFloat4x4(&cb.matWorld, DirectX::XMMatrixTranspose(matWorld));
        m_pConstantBuffer->Update(0, &cb, sizeof(cb));

        commandList->SetGraphicsRootConstantBufferView(0, m_pConstantBuffer->GetGPUVirtualAddress(0));
        
        auto pMesh = CPrimitiveGenerator::GetInstance().GetPrimitiveMesh(L"Cube");
        if (pMesh)
        {
            pMesh->Render(commandList);
        }
    }

    // ==========================================
    // Pass 2: Lighting Pass
    // ==========================================
    m_pDevice->TransitionGBuffersToPixelShaderResource();
    m_pDevice->SetMainRenderTarget();

    commandList->SetPipelineState(m_pipelineStateLighting.Get());
    commandList->SetGraphicsRootSignature(m_rootSignatureLighting.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Setup Lighting constants
    SceneConstantBuffer cb = {};
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
    
    // We can reuse objIndex or just an offset. Let's use 1023
    m_pConstantBuffer->Update(1023, &cb, sizeof(cb));
    commandList->SetGraphicsRootConstantBufferView(0, m_pConstantBuffer->GetGPUVirtualAddress(1023));

    // Bind SRVs
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_pDevice->GetGBufferSrvHeap() };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    commandList->SetGraphicsRootDescriptorTable(1, m_pDevice->GetGBufferSrvHeap()->GetGPUDescriptorHandleForHeapStart());

    // Draw full-screen quad (3 vertices generated in VS)
    commandList->DrawInstanced(3, 1, 0, 0);

    // ==========================================
    // Pass 3: Forward Rendering (Gizmos)
    // ==========================================
    if (pScene)
    {
        RenderGizmo(pGizmo, pSelectedObj);
    }

    m_pDevice->SubmitRender();
}

void CGraphicsEngine::RenderDebugGBuffers()
{
    if (!m_bIsInitialized) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_pDevice->PrepareDebugRender()) return;

    auto commandList = m_pDevice->GetCommandList();

    // Use Debug Pipeline State
    commandList->SetPipelineState(m_pipelineStateDebug.Get());
    commandList->SetGraphicsRootSignature(m_rootSignatureDebug.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Bind SRVs
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_pDevice->GetGBufferSrvHeap() };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    
    // The Debug Root Signature expects the SRV table at index 0 because there's no CBV
    commandList->SetGraphicsRootDescriptorTable(0, m_pDevice->GetGBufferSrvHeap()->GetGPUDescriptorHandleForHeapStart());

    // Draw full-screen quad
    commandList->DrawInstanced(3, 1, 0, 0);

    m_pDevice->SubmitDebugRender();
}

void CGraphicsEngine::Resize(int width, int height)
{
    if (!m_bIsInitialized) return;
    if (width == 0 || height == 0) return;
    if (m_nWidth == width && m_nHeight == height) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    m_nWidth = width;
    m_nHeight = height;

    m_pDevice->Resize(width, height);
    CPickingSystem::GetInstance().Resize(m_pDevice->GetDevice(), width, height);
}

bool CGraphicsEngine::InitializeDebugSwapChain(HWND hWnd, int width, int height)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_pDevice) return false;
    return m_pDevice->InitializeDebugSwapChain(hWnd, width, height);
}

void CGraphicsEngine::ResizeDebugSwapChain(int width, int height)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pDevice)
    {
        m_pDevice->ResizeDebugSwapChain(width, height);
    }
}



void CGraphicsEngine::RenderGameObject(std::shared_ptr<CGameObject> pObj, int& objIndex, CLight* pLight)
{
    if (objIndex >= 1024) return;

    auto pRenderer = pObj->GetComponent<CMeshRenderer>();
    if (pRenderer && pRenderer->m_bIsEnabled)
    {
        auto pFilter = pObj->GetComponent<CMeshFilter>();
        if (pFilter)
        {
            auto pTransform = pObj->GetTransform();
            if (pTransform)
            {
                // 행렬 계산
                DirectX::XMMATRIX matWorld = pTransform->GetWorldMatrix();
                
                DirectX::XMMATRIX matView;
                {
                    std::lock_guard<std::mutex> camLock(CSceneManager::GetInstance().GetCameraMutex());
                    matView = CSceneManager::GetInstance().GetEditorCamera().GetViewMatrix();
                }

                float aspectRatio = static_cast<float>(m_nWidth) / static_cast<float>(m_nHeight);
                DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 100.0f);

                DirectX::XMMATRIX matWVP = matWorld * matView * matProj;

                // 상수 버퍼 업데이트
                SceneConstantBuffer cb = {};
                DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixTranspose(matWVP));
                DirectX::XMStoreFloat4x4(&cb.matWorld, DirectX::XMMatrixTranspose(matWorld));
                cb.objectColorID = { 0.0f, 0.0f, 0.0f, 0.0f };
                cb.meshColor = { 0.0f, 0.0f, 0.0f, 0.0f }; // 기본적으로 무시

                if (pLight)
                {
                    // 빛의 방향은 Transform의 Rotation에 의해 결정되는 전방 벡터(Z축) 사용
                    DirectX::XMMATRIX lightWorld = pLight->GetOwner()->GetTransform()->GetWorldMatrix();
                    DirectX::XMVECTOR lightForward = DirectX::XMVector3Normalize(lightWorld.r[2]);
                    DirectX::XMStoreFloat4(&cb.lightDir, lightForward);
                    cb.lightColor = pLight->m_vLightColor;
                    cb.ambientColor = pLight->m_vAmbientColor;
                }
                else
                {
                    // 기본 빛
                    cb.lightDir = { 0.0f, -1.0f, 1.0f, 0.0f };
                    cb.lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
                    cb.ambientColor = { 0.2f, 0.2f, 0.2f, 1.0f };
                }

                m_pConstantBuffer->Update(objIndex, &cb, sizeof(cb));

                // 해당 오브젝트의 CBV 연결 및 그리기
                m_pDevice->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_pConstantBuffer->GetGPUVirtualAddress(objIndex));

                auto pMesh = CPrimitiveGenerator::GetInstance().GetPrimitiveMesh(pFilter->m_strMeshName);
                if (pMesh)
                {
                    pMesh->Render(m_pDevice->GetCommandList());
                }

                objIndex++;
            }
        }
    }

    // 자식들도 렌더링
    for (auto& pChild : pObj->GetChildren())
    {
        RenderGameObject(pChild, objIndex, pLight);
    }
}



void CGraphicsEngine::RenderGizmo(CGizmo* pGizmo, std::shared_ptr<CGameObject> pSelectedObj)
{
    if (pGizmo && pSelectedObj)
    {
        auto commandList = m_pDevice->GetCommandList();
        
        // 기즈모 전용 PSO 설정 (깊이 테스트 무시)
        commandList->SetPipelineState(m_pipelineStateGizmo.Get());
        commandList->SetGraphicsRootSignature(m_rootSignatureDeferred.Get());
        
        pGizmo->Render(commandList, pSelectedObj.get(), m_nWidth, m_nHeight, m_pConstantBuffer.get());
    }
}

void CGraphicsEngine::WaitForPreviousFrame()
{
    m_pDevice->WaitForPreviousFrame();
}

