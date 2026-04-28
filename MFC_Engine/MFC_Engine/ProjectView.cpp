#include "pch.h"
#include "framework.h"
#include "mainfrm.h"
#include "ProjectView.h"
#include "Resource.h"
#include "MFC_Engine.h"
#include "SceneManager.h"

CProjectView::CProjectView() noexcept
{
}

CProjectView::~CProjectView()
{
}

BEGIN_MESSAGE_MAP(CProjectView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(NM_DBLCLK, 4, &CProjectView::OnTreeDoubleClick)
END_MESSAGE_MAP()

int CProjectView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!m_wndProjectView.Create(dwViewStyle, rectDummy, this, 4))
	{
		TRACE0("Project View를 만들지 못했습니다.\n");
		return -1;
	}

	m_ProjectViewImages.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 255));
	m_wndProjectView.SetImageList(&m_ProjectViewImages, TVSIL_NORMAL);

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	FillProjectView();
	AdjustLayout();

	return 0;
}

void CProjectView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CProjectView::FillProjectView()
{
	m_wndProjectView.DeleteAllItems();

	CString strAssetsPath = _T("Assets");
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strAssetsPath);
	if (!bWorking)
	{
		strAssetsPath = _T("..\\Assets");
		bWorking = finder.FindFile(strAssetsPath);
	}

	if (bWorking)
	{
		m_strAssetsRoot = strAssetsPath;
		HTREEITEM hRoot = m_wndProjectView.InsertItem(_T("Assets"), 0, 0);
		PopulateDirectoryTree(strAssetsPath, hRoot);
		m_wndProjectView.Expand(hRoot, TVE_EXPAND);
	}
	else
	{
		m_wndProjectView.InsertItem(_T("Assets (Not Found)"), 0, 0);
	}
}

void CProjectView::PopulateDirectoryTree(const CString& strDirPath, HTREEITEM hParent)
{
	CFileFind finder;
	CString strSearchPath = strDirPath + _T("\\*.*");
	BOOL bWorking = finder.FindFile(strSearchPath);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if (finder.IsDots())
			continue;

		CString strFileName = finder.GetFileName();

		if (finder.IsDirectory())
		{
			HTREEITEM hFolder = m_wndProjectView.InsertItem(strFileName, 0, 0, hParent);
			PopulateDirectoryTree(finder.GetFilePath(), hFolder);
			// m_wndProjectView.Expand(hFolder, TVE_EXPAND); // 폴더는 기본으로 닫아두기
		}
		else
		{
			// 파일의 경우 1번 아이콘 (파일 아이콘) 사용
			m_wndProjectView.InsertItem(strFileName, 1, 1, hParent);
		}
	}
}

void CProjectView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndProjectView;
	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);
		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != nullptr) pWndTree->SelectItem(hTreeItem);
	}

	pWndTree->SetFocus();
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EXPLORER, point.x, point.y, this, TRUE);
}

void CProjectView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr) return;

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndProjectView.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CProjectView::OnPaint()
{
	CPaintDC dc(this);
	CRect rectTree;
	m_wndProjectView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);
	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CProjectView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndProjectView.SetFocus();
}

void CProjectView::OnChangeVisualStyle()
{
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE);
	m_ProjectViewImages.DeleteImageList();
	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;
	CBitmap bmp;
	if (bmp.LoadBitmap(uiBmpId))
	{
		BITMAP bmpObj;
		bmp.GetBitmap(&bmpObj);
		UINT nFlags = ILC_MASK | (theApp.m_bHiColorIcons ? ILC_COLOR24 : ILC_COLOR4);
		m_ProjectViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
		m_ProjectViewImages.Add(&bmp, RGB(255, 0, 255));
		m_wndProjectView.SetImageList(&m_ProjectViewImages, TVSIL_NORMAL);
	}
}

void CProjectView::OnTreeDoubleClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	HTREEITEM hItem = m_wndProjectView.GetSelectedItem();
	if (hItem == nullptr)
		return;

	// 파일인지 확인 (아이콘이 1이면 파일로 간주)
	int nImage, nSelectedImage;
	m_wndProjectView.GetItemImage(hItem, nImage, nSelectedImage);
	if (nImage != 1) return;

	CString strFilename = m_wndProjectView.GetItemText(hItem);

	// 확장자가 .json인지 확인
	if (strFilename.Right(5).CompareNoCase(_T(".json")) != 0)
		return;

	// 루트까지 올라가면서 상대 경로 구성
	CString strRelativePath = strFilename;
	HTREEITEM hParent = m_wndProjectView.GetParentItem(hItem);
	while (hParent != nullptr && hParent != m_wndProjectView.GetRootItem())
	{
		strRelativePath = m_wndProjectView.GetItemText(hParent) + _T("\\") + strRelativePath;
		hParent = m_wndProjectView.GetParentItem(hParent);
	}

	CString strFullPath = m_strAssetsRoot + _T("\\") + strRelativePath;

	if (CSceneManager::GetInstance().LoadScene(strFullPath.GetString()))
	{
		CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
		if (pMainFrame)
		{
			pMainFrame->OnSceneLoaded();
		}
	}
}
