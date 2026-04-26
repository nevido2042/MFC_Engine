#include "pch.h"
#include "framework.h"
#include "SceneView.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSceneView::CSceneView() noexcept
{
}

CSceneView::~CSceneView()
{
}

BEGIN_MESSAGE_MAP(CSceneView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

int CSceneView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CSceneView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
}

void CSceneView::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(rect);

	dc.FillSolidRect(rect, RGB(56, 56, 56)); // 유니티 다크 테마 배경색 느낌
	
	dc.SetTextColor(RGB(200, 200, 200));
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(_T("Scene View"), rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CSceneView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
}
