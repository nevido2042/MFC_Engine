#include "pch.h"
#include "framework.h"
#include "SceneView.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSceneView::CSceneView() noexcept
	: m_bIsRunning(false)
{
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

void CSceneView::RenderLoop()
{
	while (m_bIsRunning)
	{
		if (m_pEngine)
		{
			m_pEngine->Render();
		}
		
		// CPU 점유율 조절을 위해 아주 짧게 대기 (선택 사항)
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
