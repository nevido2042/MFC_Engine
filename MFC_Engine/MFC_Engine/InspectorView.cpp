#include "pch.h"
#include "framework.h"

#include "InspectorView.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MFC_Engine.h"
#include "MeshFilter.h"
#include "MeshRenderer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CInspectorView

CInspectorView::CInspectorView() noexcept
{
	m_nComboHeight = 0;
}

CInspectorView::~CInspectorView()
{
}

BEGIN_MESSAGE_MAP(CInspectorView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_INSPECTOR_ADD_MESH_FILTER, OnAddMeshFilter)
	ON_COMMAND(ID_INSPECTOR_ADD_MESH_RENDERER, OnAddMeshRenderer)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInspectorView 메시지 처리기

void CInspectorView::AdjustLayout()
{
	if (GetSafeHwnd () == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), m_nComboHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top + m_nComboHeight, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(nullptr, rectClient.left, rectClient.top + m_nComboHeight + cyTlb, rectClient.Width(), rectClient.Height() -(m_nComboHeight+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

int CInspectorView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 콤보 상자를 만듭니다.
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	{
		TRACE0("속성 콤보 상자를 만들지 못했습니다. \n");
		return -1;      // 만들지 못했습니다.
	}

	m_wndObjectCombo.SetCurSel(0);

	CRect rectCombo;
	m_wndObjectCombo.GetClientRect (&rectCombo);

	m_nComboHeight = rectCombo.Height();

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("속성 표를 만들지 못했습니다. \n");
		return -1;      // 만들지 못했습니다.
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* 잠금 */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* 잠금 */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// 모든 명령은 부모 프레임이 아닌 이 컨트롤을 통해 라우팅됩니다.
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CInspectorView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CInspectorView::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CInspectorView::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CInspectorView::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CInspectorView::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CInspectorView::OnProperties1()
{
}

void CInspectorView::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
}

void CInspectorView::OnProperties2()
{
}

void CInspectorView::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
}

void CInspectorView::SetSelectedGameObject(std::shared_ptr<CGameObject> pObj)
{
	m_pSelectedObj = pObj;
	if (m_pSelectedObj)
	{
		m_wndObjectCombo.ResetContent();
		m_wndObjectCombo.AddString(m_pSelectedObj->GetName().c_str());
		m_wndObjectCombo.SetCurSel(0);
	}
	
	InitPropList(); // 리스트 갱신
}

void CInspectorView::InitPropList()
{
	SetPropListFont();

	m_wndPropList.RemoveAll(); // 기존 항목 모두 삭제
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

	if (!m_pSelectedObj) return;

	auto transform = m_pSelectedObj->GetTransform();
	if (!transform) return;

	// --- Transform Component ---
	CMFCPropertyGridProperty* pTransformGroup = new CMFCPropertyGridProperty(_T("Transform"));

	// Position
	CMFCPropertyGridProperty* pPos = new CMFCPropertyGridProperty(_T("Position"), 0, TRUE);
	pPos->AddSubItem(new CMFCPropertyGridProperty(_T("X"), (_variant_t)transform->m_position.x, _T("Position X")));
	pPos->AddSubItem(new CMFCPropertyGridProperty(_T("Y"), (_variant_t)transform->m_position.y, _T("Position Y")));
	pPos->AddSubItem(new CMFCPropertyGridProperty(_T("Z"), (_variant_t)transform->m_position.z, _T("Position Z")));
	pTransformGroup->AddSubItem(pPos);

	// Rotation
	CMFCPropertyGridProperty* pRot = new CMFCPropertyGridProperty(_T("Rotation"), 0, TRUE);
	pRot->AddSubItem(new CMFCPropertyGridProperty(_T("X"), (_variant_t)transform->m_rotation.x, _T("Rotation X")));
	pRot->AddSubItem(new CMFCPropertyGridProperty(_T("Y"), (_variant_t)transform->m_rotation.y, _T("Rotation Y")));
	pRot->AddSubItem(new CMFCPropertyGridProperty(_T("Z"), (_variant_t)transform->m_rotation.z, _T("Rotation Z")));
	pTransformGroup->AddSubItem(pRot);

	// Scale
	CMFCPropertyGridProperty* pScale = new CMFCPropertyGridProperty(_T("Scale"), 0, TRUE);
	pScale->AddSubItem(new CMFCPropertyGridProperty(_T("X"), (_variant_t)transform->m_scale.x, _T("Scale X")));
	pScale->AddSubItem(new CMFCPropertyGridProperty(_T("Y"), (_variant_t)transform->m_scale.y, _T("Scale Y")));
	pScale->AddSubItem(new CMFCPropertyGridProperty(_T("Z"), (_variant_t)transform->m_scale.z, _T("Scale Z")));
	pTransformGroup->AddSubItem(pScale);

	m_wndPropList.AddProperty(pTransformGroup);
	pTransformGroup->Expand(TRUE);

	// --- Mesh Filter ---
	auto meshFilter = m_pSelectedObj->GetComponent<CMeshFilter>();
	if (meshFilter)
	{
		CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("Mesh Filter"));
		
		CMFCPropertyGridProperty* pMeshProp = new CMFCPropertyGridProperty(_T("Mesh"), (_variant_t)meshFilter->m_meshName.c_str(), _T("Select Mesh Type"));
		pMeshProp->AddOption(_T("Cube"));
		pMeshProp->AddOption(_T("Capsule"));
		pMeshProp->AddOption(_T("Sphere"));
		pMeshProp->AddOption(_T("Quad"));
		pMeshProp->AllowEdit(FALSE); // 드롭다운 선택만 가능하도록 설정

		pGroup->AddSubItem(pMeshProp);
		m_wndPropList.AddProperty(pGroup);
		pGroup->Expand(TRUE);
	}

	// --- Mesh Renderer ---
	auto meshRenderer = m_pSelectedObj->GetComponent<CMeshRenderer>();
	if (meshRenderer)
	{
		CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty(_T("Mesh Renderer"));
		pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Enabled"), (_variant_t)meshRenderer->m_isEnabled, _T("Enable/Disable Rendering")));
		m_wndPropList.AddProperty(pGroup);
		pGroup->Expand(TRUE);
	}
}

LRESULT CInspectorView::OnPropertyChanged(WPARAM, LPARAM lParam)
{
	CMFCPropertyGridProperty* pProp = (CMFCPropertyGridProperty*)lParam;
	if (!pProp || !m_pSelectedObj) return 0;

	auto transform = m_pSelectedObj->GetTransform();
	if (!transform) return 0;

	CString name = pProp->GetName();
	_variant_t value = pProp->GetValue();

	// 부모 속성을 확인하여 어떤 컴포넌트의 어떤 필드인지 확인
	CMFCPropertyGridProperty* pParent = pProp->GetParent();
	if (pParent)
	{
		CString parentName = pParent->GetName();
		if (parentName == _T("Position"))
		{
			if (name == _T("X")) transform->m_position.x = (float)value;
			else if (name == _T("Y")) transform->m_position.y = (float)value;
			else if (name == _T("Z")) transform->m_position.z = (float)value;
		}
		else if (parentName == _T("Rotation"))
		{
			if (name == _T("X")) transform->m_rotation.x = (float)value;
			else if (name == _T("Y")) transform->m_rotation.y = (float)value;
			else if (name == _T("Z")) transform->m_rotation.z = (float)value;
		}
		else if (parentName == _T("Scale"))
		{
			if (name == _T("X")) transform->m_scale.x = (float)value;
			else if (name == _T("Y")) transform->m_scale.y = (float)value;
			else if (name == _T("Z")) transform->m_scale.z = (float)value;
		}
	}

	// Mesh Filter
	auto meshFilter = m_pSelectedObj->GetComponent<CMeshFilter>();
	if (meshFilter && name == _T("Mesh"))
	{
		meshFilter->m_meshName = (LPCTSTR)(_bstr_t)value;
	}

	// Mesh Renderer
	auto meshRenderer = m_pSelectedObj->GetComponent<CMeshRenderer>();
	if (meshRenderer && name == _T("Enabled"))
	{
		meshRenderer->m_isEnabled = (bool)value;
	}

	return 0;
}

void CInspectorView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CInspectorView::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CInspectorView::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
	m_wndObjectCombo.SetFont(&m_fntPropList);
}

void CInspectorView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (!m_pSelectedObj) return;

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_INSPECTOR_ADD_MESH_FILTER, _T("Add Mesh Filter"));
	menu.AppendMenu(MF_STRING, ID_INSPECTOR_ADD_MESH_RENDERER, _T("Add Mesh Renderer"));

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

void CInspectorView::OnAddMeshFilter()
{
	if (m_pSelectedObj && !m_pSelectedObj->GetComponent<CMeshFilter>())
	{
		m_pSelectedObj->AddComponent<CMeshFilter>();
		InitPropList();
	}
}

void CInspectorView::OnAddMeshRenderer()
{
	if (m_pSelectedObj && !m_pSelectedObj->GetComponent<CMeshRenderer>())
	{
		m_pSelectedObj->AddComponent<CMeshRenderer>();
		InitPropList();
	}
}
