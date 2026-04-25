
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
END_MESSAGE_MAP()

// CMFCEngineView 생성/소멸

CMFCEngineView::CMFCEngineView() noexcept
{
	// TODO: 여기에 생성 코드를 추가합니다.

}

CMFCEngineView::~CMFCEngineView()
{
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

	// DX12 렌더링 호출
	if (m_graphicsEngine)
	{
		m_graphicsEngine->Render();
	}
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


// CMFCEngineView 메시지 처리기
