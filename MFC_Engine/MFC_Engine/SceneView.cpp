#include "pch.h"
#include "framework.h"
#include "SceneView.h"
#include "MainFrm.h"
#include "Resource.h"
#include "SceneManager.h"
#include "HierarchyView.h"
#include "InspectorView.h"
#include "GameObject.h"
#include "Transform.h"
// #include "Gizmo.h" // Legacy Gizmo Removed
#include "PickingSystem.h"
#include "MFC_Engine.h"
#include "RenderPassFactory.h"
#include "RenderPass.h"
#include "PrimitiveGenerator.h"
#include "SwapChain.h"
#include "GBuffer.h"
#include "ImGuiManager.h"
#include "ImGui/imgui.h"
#include "ImGui/ImGuizmo.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSceneView::CSceneView() noexcept
    : m_bIsRunning(false)
    , m_bRButtonDown(false)
    , m_bLButtonDown(false)
    , m_gizmoAxis(0)
{
    memset(m_keys, 0, sizeof(m_keys));
}

CSceneView::~CSceneView()
{
    m_bIsRunning = false;
    if (m_renderThread.joinable())
    {
        m_renderThread.join();
    }
}

BEGIN_MESSAGE_MAP(CSceneView, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_SETFOCUS()
    ON_WM_DESTROY()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

LRESULT CSceneView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui::GetCurrentContext() != nullptr)
    {
        if (ImGui_ImplWin32_WndProcHandler(m_hWnd, message, wParam, lParam))
            return true;

        ImGuiIO& io = ImGui::GetIO();
        bool bIsMouseMsg = (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST);
        if (io.WantCaptureMouse && bIsMouseMsg)
        {
            // Allow mouse move to pass through so MFC knows where the mouse is if needed, but block clicks
            if (message != WM_MOUSEMOVE)
                return true;
        }
        
        bool bIsKeyMsg = (message >= WM_KEYFIRST && message <= WM_KEYLAST);
        if (io.WantCaptureKeyboard && bIsKeyMsg)
        {
            return true;
        }
    }

    return CDockablePane::WindowProc(message, wParam, lParam);
}

int CSceneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    // Graphics Engine 초기화
    m_pEngine = std::make_unique<CGraphicsEngine>();

    CRect rect;
    GetClientRect(rect);
    
    // 3. 엔진 초기화 (디바이스 주입)
    CDevice* pDevice = theApp.GetDevice();
    m_pEngine->Initialize(pDevice, rect.Width(), rect.Height());

    // 4. 주요 컴포넌트 주입 (Dependency Injection)
    auto pSwapChain = std::make_unique<CSwapChain>();
    pSwapChain->Initialize(pDevice, GetSafeHwnd(), rect.Width(), rect.Height());
    m_pEngine->SetSwapChain(std::move(pSwapChain));

    auto pGBuffer = std::make_unique<CGBuffer>();
    pGBuffer->Initialize(pDevice, rect.Width(), rect.Height());
    m_pEngine->SetGBuffer(std::move(pGBuffer));

    auto pImGuiManager = std::make_unique<CImGuiManager>();
    pImGuiManager->Initialize(GetSafeHwnd(), pDevice->GetDevice(), pDevice->GetCommandQueue(), 2);
    m_pEngine->SetImGuiManager(std::move(pImGuiManager));

    // 5. 렌더 패스 구성 (추상 팩토리 패턴 적용)
    std::unique_ptr<IRenderPassFactory> pPassFactory = 
        std::make_unique<CDX12RenderPassFactory>(pDevice->GetDevice());

    m_pEngine->RegisterRenderPass(pPassFactory->CreatePass("Geometry"));
    m_pEngine->RegisterRenderPass(pPassFactory->CreatePass("Lighting"));

    // 6. 주요 시스템 주입 (Advanced DI)
    // CPickingSystem 주입
    auto pPicking = std::make_unique<CPickingSystem>();
    pPicking->Initialize(pDevice->GetDevice(), m_pEngine->GetRootSignature(), rect.Width(), rect.Height());
    m_pEngine->SetPickingSystem(std::move(pPicking));

    // CPrimitiveGenerator 주입
    auto pGenerator = std::make_unique<CPrimitiveGenerator>();
    pGenerator->Initialize(pDevice->GetDevice());
    m_pEngine->SetPrimitiveGenerator(std::move(pGenerator));

    // 렌더링 스레드 시작
    m_bIsRunning = true;
    m_renderThread = std::thread(&CSceneView::RenderLoop, this);

    return 0;
}

void CSceneView::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);

    if (m_pEngine && cx > 0 && cy > 0)
    {
        m_pEngine->Resize(cx, cy);
    }
}

void CSceneView::OnPaint()
{
    CPaintDC dc(this);
}

void CSceneView::OnSetFocus(CWnd* pOldWnd)
{
    CDockablePane::OnSetFocus(pOldWnd);
}

void CSceneView::OnDestroy()
{
    m_bIsRunning = false;
    if (m_renderThread.joinable())
    {
        m_renderThread.join();
    }

    CDockablePane::OnDestroy();
}

void CSceneView::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_bRButtonDown = true;
    m_lastMousePos = point;
    SetCapture();
    SetFocus();
}

void CSceneView::OnRButtonUp(UINT nFlags, CPoint point)
{
    m_bRButtonDown = false;
    ReleaseCapture();
}

void CSceneView::OnLButtonDown(UINT nFlags, CPoint point)
{
    // If ImGuizmo is using the mouse, don't perform picking
    if (ImGui::GetCurrentContext() != nullptr)
    {
        if (ImGui::GetIO().WantCaptureMouse || ImGuizmo::IsOver())
        {
            m_bLButtonDown = true;
            m_lastMousePos = point;
            SetCapture();
            SetFocus();
            return;
        }
    }

    m_bLButtonDown = true;
    m_lastMousePos = point;
    SetCapture();
    SetFocus();

    bool bIsPicked = false;
    if (m_pEngine)
    {
        // 엔진에 주입된 피킹 시스템을 사용
        UINT pickedID = 0;
        if (auto pPicking = m_pEngine->GetPickingSystem())
        {
            pickedID = pPicking->Pick(point.x, point.y, m_pEngine.get());
        }

        if (pickedID > 0)
        {
            bIsPicked = true;
        }

        m_gizmoAxis = 0;
        
        // 일반 오브젝트 선택
        CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
        if (pMainFrame)
        {
            auto pScene = CSceneManager::GetInstance().GetActiveScene();
            if (pScene)
            {
                SetSelectedGameObject(pScene->FindGameObjectByID(pickedID));
                pMainFrame->GetHierarchyView()->SelectGameObject(GetSelectedGameObject());
            }
        }
    }

    if (!bIsPicked)
    {
        CWnd::OnLButtonDown(nFlags, point);
    }
}

void CSceneView::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_bLButtonDown = false;
    m_gizmoAxis = 0;
    ReleaseCapture();

    if (!GetSelectedGameObject())
    {
        CWnd::OnLButtonUp(nFlags, point);
    }
}

void CSceneView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_bRButtonDown)
    {
        float dx = static_cast<float>(point.x - m_lastMousePos.x);
        float dy = static_cast<float>(point.y - m_lastMousePos.y);

        std::lock_guard<std::mutex> lock(CSceneManager::GetInstance().GetCameraMutex());
        CSceneManager::GetInstance().GetEditorCamera().Rotate(dy, dx); // dy is pitch, dx is yaw. [ignoring loop detection]
    }

    m_lastMousePos = point;
}

void CSceneView::OnContextMenu(CWnd* pWnd, CPoint point)
{
}

BOOL CSceneView::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        m_keys[pMsg->wParam] = true;
    }
    else if (pMsg->message == WM_KEYUP)
    {
        m_keys[pMsg->wParam] = false;
    }

    return CDockablePane::PreTranslateMessage(pMsg);
}

void CSceneView::RenderLoop()
{
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_bIsRunning)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        if (m_pEngine)
        {
            ProcessInput(deltaTime);
            m_pEngine->Render(CSceneManager::GetInstance().GetActiveScene(), GetSelectedGameObject());
            
            if (m_pEngine->IsDebugViewActive())
            {
                m_pEngine->RenderDebugGBuffers();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void CSceneView::ProcessInput(float deltaTime)
{
    if (!m_bRButtonDown)
    {
        return;
    }

    float forward = 0.0f;
    float right = 0.0f;
    float up = 0.0f;

    if (m_keys['W']) forward += 1.0f;
    if (m_keys['S']) forward -= 1.0f;
    if (m_keys['A']) right -= 1.0f;
    if (m_keys['D']) right += 1.0f;
    if (m_keys['Q']) up -= 1.0f;
    if (m_keys['E']) up += 1.0f;

    if (forward != 0.0f || right != 0.0f || up != 0.0f)
    {
        std::lock_guard<std::mutex> lock(CSceneManager::GetInstance().GetCameraMutex());
        CSceneManager::GetInstance().GetEditorCamera().Move(forward, right, up, deltaTime);
    }
}
