#pragma once
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"
#include "ImGui/ImGuizmo.h"
#include <wrl.h>
#include <d3d12.h>
#include <memory>
#include <DirectXMath.h>

/**
 * @class CImGuiManager
 * @brief ImGui 및 ImGuizmo의 초기화, 렌더링, 수명 주기를 관리하는 클래스입니다.
 */
class CImGuiManager
{
public:
    CImGuiManager();
    ~CImGuiManager();

    /**
     * @brief ImGui를 초기화합니다.
     */
    bool Initialize(HWND hWnd, ID3D12Device* pDevice, ID3D12CommandQueue* pCommandQueue, int nNumFramesInFlight);
    
    /**
     * @brief ImGui 리소스를 해제합니다.
     */
    void Shutdown();

    /**
     * @brief 새로운 ImGui 프레임을 시작합니다.
     */
    void NewFrame();

    /**
     * @brief ImGui 및 ImGuizmo의 그리기를 처리합니다.
     */
    void Render(ID3D12GraphicsCommandList* pCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hRtv);

    /**
     * @brief 기즈모 조작 및 그리드를 처리합니다.
     */
    void UpdateGizmo(class CGameObject* pSelectedObj, const DirectX::XMMATRIX& matView, const DirectX::XMMATRIX& matProj, int nWidth, int nHeight);

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pSrvHeap;
    bool m_bIsInitialized;

    // 기즈모 상태 관리
    ImGuizmo::OPERATION m_currentGizmoOperation;
    ImGuizmo::MODE      m_currentGizmoMode;

    struct ImGuiContext* m_pContext;
};
