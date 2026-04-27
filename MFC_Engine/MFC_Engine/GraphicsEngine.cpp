#include "pch.h"
#include "d3dx12.h"
#include "GraphicsEngine.h"
#include <d3dcompiler.h>
#include "Scene.h"
#include "Transform.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "PickingSystem.h"
#include "SceneManager.h"

// 쉐이더 컴파일 라이브러리 링크
#pragma comment(lib, "d3dcompiler.lib")

CGraphicsEngine::CGraphicsEngine()
    : m_isInitialized(false)
    , m_width(0)
    , m_height(0)
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
    m_width = (width > 0) ? width : 1;
    m_height = (height > 0) ? height : 1;

    m_timeManager.Initialize();

    m_pDevice = std::make_unique<CDevice>();
    m_pDevice->Initialize(hWnd, m_width, m_height);

    CreateRootSignature();
    CreatePipelineState();

    CPickingSystem::GetInstance().Initialize(m_pDevice->GetDevice(), m_rootSignature, m_width, m_height);
    CPrimitiveGenerator::GetInstance().Initialize(m_pDevice->GetDevice());
    CreateConstantBuffer();

    m_isInitialized = true;
    return true;
}


/**
 * @brief 쉐이더 자원 바인딩을 위한 루트 시그니처를 생성합니다.
 */
void CGraphicsEngine::CreateRootSignature()
{
    // 상수 버퍼를 위한 루트 파라미터 정의
    D3D12_ROOT_PARAMETER rootParameters[1];
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    rootParameters[0].Descriptor.RegisterSpace = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // 기본 루트 시그니처 구조체 직접 작성
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(m_pDevice->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

/**
 * @brief 그래픽 파이프라인 상태 객체(PSO)를 생성합니다.
 */
void CGraphicsEngine::CreatePipelineState()
{
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    // 쉐이더 파일 컴파일
    ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

    // 입력 레이아웃 정의
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 파이프라인 상태 객체(PSO) 상세 설정 (Raw Struct 직접 채우기)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    
    // 기본 래스터라이저 설정
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

    // 기본 블렌드 설정
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

    // 깊이 스텐실 설정
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

    ThrowIfFailed(m_pDevice->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}



/**
 * @brief 쉐이더 상수를 담을 버퍼를 생성하고 메모리 매핑을 수행합니다.
 */
void CGraphicsEngine::CreateConstantBuffer()
{
    m_pConstantBuffer = std::make_unique<CConstantBuffer>();
    m_pConstantBuffer->Initialize(m_pDevice->GetDevice(), sizeof(SceneConstantBuffer), 1024);
}


/**
 * @brief 실시간 렌더링 루프를 수행합니다.
 */
void CGraphicsEngine::Render()
{
    Render(nullptr);
}

void CGraphicsEngine::Render(std::shared_ptr<CScene> pScene)
{
    if (!m_isInitialized) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_timeManager.Update();

    m_pDevice->PrepareRender();
    
    auto commandList = m_pDevice->GetCommandList();
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (pScene)
    {
        const auto& gameObjects = pScene->GetGameObjects();
        int objIndex = 0;
        for (auto& pObj : gameObjects)
        {
            RenderGameObject(pObj, objIndex);
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

        float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
        DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 100.0f);
        DirectX::XMMATRIX matWVP = matWorld * matView * matProj;

        SceneConstantBuffer cb;
        DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixTranspose(matWVP));
        m_pConstantBuffer->Update(0, &cb, sizeof(cb));

        commandList->SetGraphicsRootConstantBufferView(0, m_pConstantBuffer->GetGPUVirtualAddress(0));
        
        auto pMesh = CPrimitiveGenerator::GetInstance().GetPrimitiveMesh(L"Cube");
        if (pMesh)
        {
            pMesh->Render(commandList);
        }
    }

    m_pDevice->SubmitRender();
}

void CGraphicsEngine::Resize(int width, int height)
{
    if (!m_isInitialized) return;
    if (width == 0 || height == 0) return;
    if (m_width == width && m_height == height) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    m_width = width;
    m_height = height;

    m_pDevice->Resize(width, height);
    CPickingSystem::GetInstance().Resize(m_pDevice->GetDevice(), width, height);
}



void CGraphicsEngine::RenderGameObject(std::shared_ptr<CGameObject> pObj, int& objIndex)
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
                // 행렬 계산
                DirectX::XMMATRIX matWorld = pTransform->GetWorldMatrix();
                
                DirectX::XMMATRIX matView;
                {
                    std::lock_guard<std::mutex> camLock(CSceneManager::GetInstance().GetCameraMutex());
                    matView = CSceneManager::GetInstance().GetEditorCamera().GetViewMatrix();
                }

                float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
                DirectX::XMMATRIX matProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 100.0f);

                DirectX::XMMATRIX matWVP = matWorld * matView * matProj;

                // 상수 버퍼 업데이트
                SceneConstantBuffer cb;
                DirectX::XMStoreFloat4x4(&cb.matWVP, DirectX::XMMatrixTranspose(matWVP));
                m_pConstantBuffer->Update(objIndex, &cb, sizeof(cb));

                // 해당 오브젝트의 CBV 연결 및 그리기
                m_pDevice->GetCommandList()->SetGraphicsRootConstantBufferView(0, m_pConstantBuffer->GetGPUVirtualAddress(objIndex));

                auto pMesh = CPrimitiveGenerator::GetInstance().GetPrimitiveMesh(pFilter->m_meshName);
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
        RenderGameObject(pChild, objIndex);
    }
}


UINT CGraphicsEngine::Pick(int x, int y)
{
    if (!m_isInitialized) return 0;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_pDevice->WaitForPreviousFrame();

    UINT id = CPickingSystem::GetInstance().Pick(x, y, 
        m_pDevice->GetDevice(), 
        m_pDevice->GetCommandQueue(), 
        m_pDevice->GetCommandAllocator(), 
        m_pDevice->GetCommandList(), 
        m_rootSignature.Get(), 
        m_pConstantBuffer->GetResource(), 
        m_pConstantBuffer->GetMappedData(), 
        m_width, m_height);
    
    m_pDevice->WaitForPreviousFrame();

    return id;
}

void CGraphicsEngine::WaitForPreviousFrame()
{
    m_pDevice->WaitForPreviousFrame();
}

