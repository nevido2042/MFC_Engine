#include "pch.h"
#include "ImGuiManager.h"
#include "GameObject.h"
#include "Transform.h"
#include "SceneManager.h"
#include "Camera.h"

CImGuiManager::CImGuiManager()
    : m_bIsInitialized(false)
    , m_currentGizmoOperation(ImGuizmo::TRANSLATE)
    , m_currentGizmoMode(ImGuizmo::LOCAL)
{
}

CImGuiManager::~CImGuiManager()
{
    Shutdown();
}

bool CImGuiManager::Initialize(HWND hWnd, ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue, int nNumFramesInFlight)
{
    // 1. Descriptor Heap 생성 (ImGui 전역 폰트/이미지용)
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 128;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    
    if (FAILED(pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pSrvHeap))))
        return false;

    // 2. ImGui Context 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // MFC 호환성 문제로 비활성화

    ImGui::StyleColorsDark();

    // 3. Backend 초기화
    ImGui_ImplWin32_Init(hWnd);

    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device = pDevice;
    initInfo.CommandQueue = pCommandQueue;
    initInfo.NumFramesInFlight = nNumFramesInFlight;
    initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    initInfo.SrvDescriptorHeap = m_pSrvHeap.Get();
    initInfo.LegacySingleSrvCpuDescriptor = m_pSrvHeap->GetCPUDescriptorHandleForHeapStart();
    initInfo.LegacySingleSrvGpuDescriptor = m_pSrvHeap->GetGPUDescriptorHandleForHeapStart();
    
    if (!ImGui_ImplDX12_Init(&initInfo))
        return false;

    m_bIsInitialized = true;
    return true;
}

void CImGuiManager::Shutdown()
{
    if (m_bIsInitialized)
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        m_bIsInitialized = false;
    }
}

void CImGuiManager::NewFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void CImGuiManager::Render(ID3D12GraphicsCommandList* pCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hRtv)
{
    ImGui::Render();

    // 렌더 타겟 설정 (ImGui 그리기 직전)
    pCommandList->OMSetRenderTargets(1, &hRtv, FALSE, nullptr);

    // Descriptor Heap 설정
    ID3D12DescriptorHeap* ppHeaps[] = { m_pSrvHeap.Get() };
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // 실제 그리기
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);
}

void CImGuiManager::UpdateGizmo(CGameObject* pSelectedObj, const DirectX::XMMATRIX& matView, const DirectX::XMMATRIX& matProj, int nWidth, int nHeight)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
    ImGuizmo::SetRect(0, 0, (float)nWidth, (float)nHeight);

    // 1. 그리드 그리기 (디버그용)
    {
        DirectX::XMFLOAT4X4 view, proj;
        DirectX::XMStoreFloat4x4(&view, matView);
        DirectX::XMStoreFloat4x4(&proj, matProj);

        DirectX::XMMATRIX matIdentity = DirectX::XMMatrixIdentity();
        DirectX::XMFLOAT4X4 ident;
        DirectX::XMStoreFloat4x4(&ident, matIdentity);

        ImGuizmo::DrawGrid(&view.m[0][0], &proj.m[0][0], &ident.m[0][0], 100.f);
    }

    // 2. 기즈모 조작
    if (pSelectedObj)
    {
        DirectX::XMFLOAT4X4 view, proj;
        DirectX::XMStoreFloat4x4(&view, matView);
        DirectX::XMStoreFloat4x4(&proj, matProj);

        CTransform* pTransform = pSelectedObj->GetTransform().get();
        DirectX::XMMATRIX matWorld = pTransform->GetWorldMatrix();
        DirectX::XMFLOAT4X4 world;
        DirectX::XMStoreFloat4x4(&world, matWorld);

        ImGuizmo::SetOrthographic(false);

        // 단축키 처리
        if (!io.WantCaptureKeyboard)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_W)) m_currentGizmoOperation = ImGuizmo::TRANSLATE;
            if (ImGui::IsKeyPressed(ImGuiKey_E)) m_currentGizmoOperation = ImGuizmo::ROTATE;
            if (ImGui::IsKeyPressed(ImGuiKey_R)) m_currentGizmoOperation = ImGuizmo::SCALE;
        }

        ImGuizmo::Manipulate(&view.m[0][0], &proj.m[0][0], m_currentGizmoOperation, m_currentGizmoMode, &world.m[0][0]);

        if (ImGuizmo::IsUsing())
        {
            DirectX::XMMATRIX matModifiedWorld = DirectX::XMLoadFloat4x4(&world);
            DirectX::XMVECTOR vScale, qRot, vPos;
            DirectX::XMMatrixDecompose(&vScale, &qRot, &vPos, matModifiedWorld);

            DirectX::XMStoreFloat3(&pTransform->m_vPosition, vPos);
            // 필요시 회전 및 스케일도 업데이트 가능
        }
    }
}
