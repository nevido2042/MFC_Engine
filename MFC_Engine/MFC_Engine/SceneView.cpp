#include "pch.h"
#include "framework.h"
#include "SceneView.h"
#include "Resource.h"
#include "MainFrm.h"
#include "SceneManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSceneView::CSceneView() noexcept
	: m_bIsRunning(false)
	, m_bRButtonDown(false)
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
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

int CSceneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Graphics Engine 초기화
	m_pEngine = std::make_unique<CGraphicsEngine>();
	
	CRect rect;
	GetClientRect(rect);
	m_pEngine->Initialize(GetSafeHwnd(), rect.Width(), rect.Height());

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
	// 스레드에서 렌더링하므로 GDI 그리기는 생략하거나 배경만 지움
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
	CDockablePane::OnRButtonDown(nFlags, point);
}

void CSceneView::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bRButtonDown = false;
	ReleaseCapture();
	CDockablePane::OnRButtonUp(nFlags, point);
}

void CSceneView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_pEngine)
	{
		UINT pickedID = m_pEngine->Pick(point.x, point.y);

		CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
		if (pMainFrame)
		{
			auto pScene = CSceneManager::GetInstance().GetActiveScene();
			if (pScene)
			{
				auto pObj = pScene->FindGameObjectByID(pickedID);
				pMainFrame->GetHierarchyView()->SelectGameObject(pObj);
			}
		}
	}
	CDockablePane::OnLButtonDown(nFlags, point);
}

void CSceneView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bRButtonDown && m_pEngine)
	{
		float dx = static_cast<float>(point.x - m_lastMousePos.x);
		float dy = static_cast<float>(point.y - m_lastMousePos.y);

		m_pEngine->RotateCamera(dy, dx);
		m_lastMousePos = point;
	}

	CDockablePane::OnMouseMove(nFlags, point);
}

void CSceneView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// 컨텍스트 메뉴가 뜨지 않도록 아무것도 하지 않음
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
			m_pEngine->Render(CSceneManager::GetInstance().GetActiveScene());
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void CSceneView::ProcessInput(float deltaTime)
{
	if (!m_bRButtonDown) return;

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
		m_pEngine->MoveCamera(forward, right, up, deltaTime);
	}
}
