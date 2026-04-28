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
#include "Gizmo.h"
#include "PickingSystem.h"

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
    m_pEngine->Initialize(GetSafeHwnd(), rect.Width(), rect.Height());

    // 기즈모 초기화 (엔진의 디바이스 사용)
    m_pGizmo = std::make_unique<CGizmo>();
    m_pGizmo->Initialize(m_pEngine->GetDevice());

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
    m_bLButtonDown = true;
    m_lastMousePos = point;
    SetCapture();
    SetFocus();

    bool bIsPicked = false;
    if (m_pEngine)
    {
        // 엔진 대신 피킹 시스템을 통해 직접 피킹 수행
        // 기즈모 렌더링 로직을 람다로 전달하여 결합도 해제
        UINT pickedID = CPickingSystem::GetInstance().Pick(point.x, point.y, m_pEngine.get());

        if (pickedID > 0)
        {
            bIsPicked = true;
        }

        // 기즈모 선택 확인
        if (pickedID >= 0xFF0001 && pickedID <= 0xFF0003)
        {
            m_gizmoAxis = pickedID - 0xFF0000;
        }
        else
        {
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
    }

    if (!bIsPicked)
    {
        CWnd::OnLButtonDown(nFlags, point);
    }
}

void CSceneView::OnLButtonUp(UINT nFlags, CPoint point)
{
    bool bWasGizmoDragging = (m_gizmoAxis > 0);
    
    m_bLButtonDown = false;
    m_gizmoAxis = 0;
    ReleaseCapture();

    if (!bWasGizmoDragging && !GetSelectedGameObject())
    {
        CWnd::OnLButtonUp(nFlags, point);
    }
}

void CSceneView::OnMouseMove(UINT nFlags, CPoint point)
{
    auto pSelected = GetSelectedGameObject();
    if (m_bRButtonDown && m_pEngine)
    {
        float dx = static_cast<float>(point.x - m_lastMousePos.x);
        float dy = static_cast<float>(point.y - m_lastMousePos.y);

        {
            std::lock_guard<std::mutex> lock(CSceneManager::GetInstance().GetCameraMutex());
            CSceneManager::GetInstance().GetEditorCamera().Rotate(dy, dx);
        }
    }
    else if (m_bLButtonDown && m_gizmoAxis > 0 && pSelected)
    {
        float dx = static_cast<float>(point.x - m_lastMousePos.x);
        float dy = static_cast<float>(point.y - m_lastMousePos.y);

        auto pTransform = pSelected->GetTransform();
        DirectX::XMFLOAT3 vPos = pTransform->m_vPosition;
        float fMoveScale = 0.02f;

        // 카메라 방향 벡터 가져오기
        DirectX::XMVECTOR camRight, camUp;
        {
            std::lock_guard<std::mutex> lock(CSceneManager::GetInstance().GetCameraMutex());
            auto& camera = CSceneManager::GetInstance().GetEditorCamera();
            camRight = camera.GetRight();
            camUp = camera.GetUp();
        }

        // 화면상 마우스 이동량을 월드 공간의 이동 방향 벡터로 변환
        // dy는 MFC 화면 좌표계(아래가 +)와 월드 좌표계(위가 +)가 반대이므로 부호 반전
        DirectX::XMVECTOR screenMove = DirectX::XMVectorAdd(
            DirectX::XMVectorScale(camRight, dx),
            DirectX::XMVectorScale(camUp, -dy)
        );

        // 현재 조작 중인 축 벡터 설정
        DirectX::XMVECTOR axisVec = DirectX::XMVectorSet(0, 0, 0, 0);
        if (m_gizmoAxis == 1)      axisVec = DirectX::XMVectorSet(1, 0, 0, 0); // X
        else if (m_gizmoAxis == 2) axisVec = DirectX::XMVectorSet(0, 1, 0, 0); // Y
        else if (m_gizmoAxis == 3) axisVec = DirectX::XMVectorSet(0, 0, 1, 0); // Z

        // 화면 이동 벡터를 해당 축에 투영하여 실제 이동량 계산
        float amount = DirectX::XMVectorGetX(DirectX::XMVector3Dot(screenMove, axisVec)) * fMoveScale;

        if (m_gizmoAxis == 1)      vPos.x += amount;
        else if (m_gizmoAxis == 2) vPos.y += amount;
        else if (m_gizmoAxis == 3) vPos.z += amount;

        pTransform->m_vPosition = vPos;

        CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
        if (pMainFrame && pMainFrame->GetInspectorView())
        {
            pMainFrame->GetInspectorView()->SetSelectedGameObject(pSelected);
        }
    }
    else if (m_bLButtonDown && m_gizmoAxis == 0)
    {
        CWnd::OnMouseMove(nFlags, point);
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
            m_pEngine->Render(CSceneManager::GetInstance().GetActiveScene(), GetSelectedGameObject(), m_pGizmo.get());
            
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
