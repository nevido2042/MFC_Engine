
#include "pch.h"
#include "framework.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "MFC_Engine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd() noexcept
{
	m_nComboHeight = 0;
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar 메시지 처리기

void CPropertiesWnd::AdjustLayout()
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

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

	// m_wndObjectCombo.AddString(_T("애플리케이션"));
	// m_wndObjectCombo.AddString(_T("속성 창"));
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

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
}

void CPropertiesWnd::OnProperties2()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
{
	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
}

void CPropertiesWnd::SetSelectedGameObject(std::shared_ptr<CGameObject> pObj)
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

void CPropertiesWnd::InitPropList()
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
}

LRESULT CPropertiesWnd::OnPropertyChanged(WPARAM, LPARAM lParam)
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

	return 0;
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont()
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
