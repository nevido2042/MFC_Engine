#include "pch.h"
#include "GBufferView.h"
#include "GraphicsEngine.h"

BEGIN_MESSAGE_MAP(CGBufferView, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

CGBufferView::CGBufferView() noexcept
    : m_pEngine(nullptr)
{
}

CGBufferView::~CGBufferView()
{
}

int CGBufferView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CGBufferView::SetEngine(CGraphicsEngine* pEngine)
{
    m_pEngine = pEngine;
    if (m_pEngine && GetSafeHwnd())
    {
        CRect rect;
        GetClientRect(&rect);
        int width = rect.Width() > 0 ? rect.Width() : 800;
        int height = rect.Height() > 0 ? rect.Height() : 600;
        m_pEngine->InitializeDebugSwapChain(GetSafeHwnd(), width, height);
    }
}

void CGBufferView::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);

    if (m_pEngine && GetSafeHwnd() && cx > 0 && cy > 0)
    {
        // Resize GBuffer debug swap chain
        m_pEngine->ResizeDebugSwapChain(cx, cy);
    }
}

void CGBufferView::OnPaint()
{
    CPaintDC dc(this);

    if (m_pEngine)
    {
        // 렌더링은 CSceneView의 메인 렌더 스레드 루프에서 주기적으로 호출됩니다.
        // OnPaint에서는 별도로 렌더링하지 않음.
    }
    else
    {
        CRect rect;
        GetClientRect(&rect);
        dc.FillSolidRect(rect, RGB(32, 32, 32));
        dc.SetTextColor(RGB(200, 200, 200));
        dc.SetBkMode(TRANSPARENT);
        dc.DrawText(_T("G-Buffer Debug View (Initializing...)"), rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

BOOL CGBufferView::OnEraseBkgnd(CDC* pDC)
{
    // Prevent flickering
    return TRUE;
}

void CGBufferView::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDockablePane::OnShowWindow(bShow, nStatus);

    if (m_pEngine)
    {
        m_pEngine->SetDebugViewActive(bShow != FALSE);
    }
}
