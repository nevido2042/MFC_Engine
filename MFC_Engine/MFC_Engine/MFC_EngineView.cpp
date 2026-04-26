
// MFC_EngineView.cpp: CMFCEngineView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "MFC_Engine.h"
#endif

#include "MFC_EngineDoc.h"
#include "MFC_EngineView.h"
#include "GraphicsEngine.h"
#include <memory>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCEngineView

IMPLEMENT_DYNCREATE(CMFCEngineView, CView)

BEGIN_MESSAGE_MAP(CMFCEngineView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMFCEngineView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CMFCEngineView 생성/소멸

CMFCEngineView::CMFCEngineView() noexcept
	: m_bRenderThreadRunning(false)
{
	// TODO: 여기에 생성 코드를 추가합니다.
}

CMFCEngineView::~CMFCEngineView()
{
	// 스레드 종료 대기
	m_bRenderThreadRunning = false;
	if (m_renderThread.joinable())
	{
		m_renderThread.join();
	}
}

BOOL CMFCEngineView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CMFCEngineView 그리기

void CMFCEngineView::OnDraw(CDC* /*pDC*/)
{
	CMFCEngineDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// 스레드에서 렌더링을 처리하므로 OnDraw에서는 별도 처리를 하지 않거나
	// 필요한 경우 한 프레임만 명시적으로 그릴 수 있습니다.
}


// CMFCEngineView 인쇄


void CMFCEngineView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMFCEngineView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CMFCEngineView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CMFCEngineView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}

void CMFCEngineView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFCEngineView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

int CMFCEngineView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Graphics Engine 생성
	m_graphicsEngine = std::make_unique<CGraphicsEngine>();

	return 0;
}

void CMFCEngineView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	CRect rect;
	GetClientRect(&rect);

	if (m_graphicsEngine)
	{
		m_graphicsEngine->Initialize(GetSafeHwnd(), rect.Width(), rect.Height());
	}

	// 렌더링 스레드 시작
	m_bRenderThreadRunning = true;
	m_renderThread = std::thread(&CMFCEngineView::RenderThreadLoop, this);

	// FPS 갱신용 타이머 (500ms마다 한 번씩 타이틀 업데이트)
	SetTimer(2, 500, NULL);
}

void CMFCEngineView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_graphicsEngine)
	{
		m_graphicsEngine->Resize(cx, cy);
	}
}

BOOL CMFCEngineView::OnEraseBkgnd(CDC* pDC)
{
	// 배경을 그리지 않음 (DX12가 채움)
	return TRUE;
}


// CMFCEngineView 진단

#ifdef _DEBUG
void CMFCEngineView::AssertValid() const
{
	CView::AssertValid();
}

void CMFCEngineView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCEngineDoc* CMFCEngineView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCEngineDoc)));
	return (CMFCEngineDoc*)m_pDocument;
}
#endif //_DEBUG



void CMFCEngineView::RenderThreadLoop()
{
	while (m_bRenderThreadRunning)
	{
		if (m_graphicsEngine)
		{
			m_graphicsEngine->Render();

			// FPS를 메인 윈도우 타이틀에 표시 (UI 스레드 작업이므로 PostMessage 고려 가능하나 간단히 직접 수행)
			// 단, AfxGetMainWnd() 등은 스레드 안전성에 주의해야 함.
			// 여기서는 엔진의 GetFPS()만 호출하고, 타이틀 업데이트는 별도 타이머나 이벤트를 권장하지만
			// 우선 렌더링 루프 확인을 위해 엔진만 호출.
		}
	}
}

void CMFCEngineView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 2) // FPS 업데이트 타이머
	{
		if (m_graphicsEngine)
		{
			float currentFPS = m_graphicsEngine->GetFPS();
			CString strFPS;
			strFPS.Format(_T("MFC_Engine [FPS: %.1f]"), currentFPS);
			
			// 메인 윈도우 또는 프레임 윈도우 타이틀 업데이트
			CFrameWnd* pParentFrame = GetParentFrame();
			if (pParentFrame)
			{
				pParentFrame->SetWindowText(strFPS);
			}
			else
			{
				AfxGetMainWnd()->SetWindowText(strFPS);
			}
		}
	}

	CView::OnTimer(nIDEvent);
}

// CMFCEngineView 메시지 처리기
