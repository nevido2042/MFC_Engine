#include "pch.h"
#include "framework.h"
#include "MainFrm.h"
#include "HierarchyView.h"
#include "Resource.h"
#include "MFC_Engine.h"
#include "SceneManager.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"

//////////////////////////////////////////////////////////////////////
// 생성/소멸
//////////////////////////////////////////////////////////////////////

CHierarchyView::CHierarchyView() noexcept
{
	m_nCurrSort = ID_SORTING_GROUPBYTYPE;
}

CHierarchyView::~CHierarchyView()
{
}

BEGIN_MESSAGE_MAP(CHierarchyView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(TVN_SELCHANGED, 2, OnSelChanged)
	ON_COMMAND(ID_HIERARCHY_CREATE_EMPTY, OnCreateEmpty)
	ON_COMMAND(ID_HIERARCHY_CREATE_CUBE, OnCreateCube)
	ON_COMMAND(ID_HIERARCHY_CREATE_PLANE, OnCreatePlane)
	ON_COMMAND_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnUpdateSort)
END_MESSAGE_MAP()

int CHierarchyView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndHierarchyView.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("Hierarchy View를 만들지 못했습니다.\n");
		return -1;
	}

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	m_wndToolBar.LoadToolBar(IDR_SORT, 0, 0, TRUE);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	FillHierarchyView();

	return 0;
}

void CHierarchyView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CHierarchyView::FillHierarchyView()
{
	m_wndHierarchyView.DeleteAllItems();
	m_mapGameObjects.clear();

	auto pScene = CSceneManager::GetInstance().GetActiveScene();
	if (!pScene) return;

	// Scene Root 추가
	CString strSceneName = pScene->GetName().c_str();
	HTREEITEM hSceneRoot = m_wndHierarchyView.InsertItem(strSceneName, 0, 0, TVI_ROOT);
	m_wndHierarchyView.SetItemState(hSceneRoot, TVIS_BOLD, TVIS_BOLD);

	auto gameObjects = pScene->GetGameObjects();

	for (auto& obj : gameObjects)
	{
		InsertGameObject(hSceneRoot, obj);
	}

	m_wndHierarchyView.Expand(hSceneRoot, TVE_EXPAND);
}

void CHierarchyView::InsertGameObject(HTREEITEM hParent, std::shared_ptr<CGameObject> pObj)
{
	if (!pObj) return;

	HTREEITEM hItem = m_wndHierarchyView.InsertItem(pObj->GetName().c_str(), 1, 1, hParent);
	m_mapGameObjects[hItem] = pObj;
	
	for (auto& child : pObj->GetChildren())
	{
		InsertGameObject(hItem, child);
	}

	m_wndHierarchyView.Expand(hItem, TVE_EXPAND);
}

void CHierarchyView::OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	HTREEITEM hSelected = pNMTreeView->itemNew.hItem;
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (!pMainFrame) return;

	if (hSelected && m_mapGameObjects.count(hSelected))
	{
		auto pObj = m_mapGameObjects[hSelected];
		pMainFrame->GetInspectorView()->SetSelectedGameObject(pObj);
	}
	else
	{
		// 씬 루트가 선택되었거나 선택이 해제된 경우
		pMainFrame->GetInspectorView()->SetSelectedGameObject(nullptr);
	}
}

void CHierarchyView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndHierarchyView;
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
		HTREEITEM hItem = pWndTree->HitTest(ptTree, &flags);
		if (hItem != nullptr) pWndTree->SelectItem(hItem);
	}

	pWndTree->SetFocus();

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_HIERARCHY_CREATE_EMPTY, _T("Create Empty GameObject"));
	
	CMenu subMenu;
	subMenu.CreatePopupMenu();
	subMenu.AppendMenu(MF_STRING, ID_HIERARCHY_CREATE_CUBE, _T("Cube"));
	subMenu.AppendMenu(MF_STRING, ID_HIERARCHY_CREATE_PLANE, _T("Plane"));
	
	menu.AppendMenu(MF_POPUP, (UINT_PTR)subMenu.Detach(), _T("Create 3D Object"));

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

void CHierarchyView::OnCreateEmpty()
{
	auto pScene = CSceneManager::GetInstance().GetActiveScene();
	if (!pScene) return;

	HTREEITEM hSelected = m_wndHierarchyView.GetSelectedItem();
	auto newObj = CGameObject::Create(L"New GameObject");

	// 씬 루트(부모가 TVI_ROOT인 첫 번째 아이템)가 선택된 경우도 처리
	if (hSelected && m_mapGameObjects.count(hSelected))
	{
		auto pParent = m_mapGameObjects[hSelected];
		pParent->AddChild(newObj);
	}
	else
	{
		// 씬 루트가 선택되었거나 아무것도 선택되지 않은 경우 씬의 최상위 오브젝트로 추가
		pScene->AddGameObject(newObj);
	}

	FillHierarchyView();
}

void CHierarchyView::OnCreateCube()
{
	auto pScene = CSceneManager::GetInstance().GetActiveScene();
	if (!pScene) return;

	HTREEITEM hSelected = m_wndHierarchyView.GetSelectedItem();
	auto newObj = CGameObject::Create(L"Cube");
	
	// Mesh 컴포넌트 추가
	auto pFilter = newObj->AddComponent<CMeshFilter>();
	pFilter->m_meshName = L"Cube";
	newObj->AddComponent<CMeshRenderer>();

	if (hSelected && m_mapGameObjects.count(hSelected))
	{
		auto pParent = m_mapGameObjects[hSelected];
		pParent->AddChild(newObj);
	}
	else
	{
		pScene->AddGameObject(newObj);
	}

	FillHierarchyView();
}

void CHierarchyView::OnCreatePlane()
{
	auto pScene = CSceneManager::GetInstance().GetActiveScene();
	if (!pScene) return;

	HTREEITEM hSelected = m_wndHierarchyView.GetSelectedItem();
	auto newObj = CGameObject::Create(L"Plane");
	
	// Mesh 컴포넌트 추가
	auto pFilter = newObj->AddComponent<CMeshFilter>();
	pFilter->m_meshName = L"Plane";
	newObj->AddComponent<CMeshRenderer>();

	if (hSelected && m_mapGameObjects.count(hSelected))
	{
		auto pParent = m_mapGameObjects[hSelected];
		pParent->AddChild(newObj);
	}
	else
	{
		pScene->AddGameObject(newObj);
	}

	FillHierarchyView();
}

void CHierarchyView::AdjustLayout()
{
	if (GetSafeHwnd() == nullptr) return;

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndHierarchyView.SetWindowPos(nullptr, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CHierarchyView::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CHierarchyView::OnSort(UINT id)
{
	if (m_nCurrSort == id) return;
	m_nCurrSort = id;
	m_wndToolBar.Invalidate();
	m_wndToolBar.UpdateWindow();
}

void CHierarchyView::OnUpdateSort(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrSort);
}

void CHierarchyView::OnPaint()
{
	CPaintDC dc(this);
	CRect rectTree;
	m_wndHierarchyView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);
	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CHierarchyView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndHierarchyView.SetFocus();
}

void CHierarchyView::OnChangeVisualStyle()
{
	m_HierarchyViewImages.DeleteImageList();
	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;
	CBitmap bmp;
	if (bmp.LoadBitmap(uiBmpId))
	{
		BITMAP bmpObj;
		bmp.GetBitmap(&bmpObj);
		UINT nFlags = ILC_MASK | (theApp.m_bHiColorIcons ? ILC_COLOR24 : ILC_COLOR4);
		m_HierarchyViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
		m_HierarchyViewImages.Add(&bmp, RGB(255, 0, 0));
		m_wndHierarchyView.SetImageList(&m_HierarchyViewImages, TVSIL_NORMAL);
	}
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE);
}
