#include "pch.h"
#include "framework.h"
#include "GameView.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGameView::CGameView() noexcept
{
}

CGameView::~CGameView()
{
}

BEGIN_MESSAGE_MAP(CGameView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

int CGameView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CGameView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
}

void CGameView::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(rect);

	dc.FillSolidRect(rect, RGB(32, 32, 32)); // 더 어두운 배경
	
	dc.SetTextColor(RGB(200, 200, 200));
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(_T("Game View"), rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CGameView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
}
