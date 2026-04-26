#pragma once

#include "ViewTree.h"

class CHierarchyToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

#include <memory>
#include <map>
#include "GameObject.h"

class CHierarchyView : public CDockablePane
{
public:
	CHierarchyView() noexcept;
	virtual ~CHierarchyView();

	void AdjustLayout();
	void OnChangeVisualStyle();

protected:
	CHierarchyToolBar m_wndToolBar;
	CViewTree m_wndHierarchyView;
	CImageList m_HierarchyViewImages;
	UINT m_nCurrSort;

	void FillHierarchyView();
	void InsertGameObject(HTREEITEM hParent, std::shared_ptr<CGameObject> pObj);
	
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCreateEmpty();
	afx_msg void OnCreateCube();
	afx_msg void OnCreatePlane();
	afx_msg void OnCreateQuad();
	afx_msg void OnCreateSphere();
	afx_msg void OnCreateCapsule();
	afx_msg void OnDelete();
	afx_msg void OnKeyDown(NMHDR* pNMHDR, LRESULT* pResult);

	void CreatePrimitive(const std::wstring& name, const std::wstring& meshName);

	std::map<HTREEITEM, std::shared_ptr<CGameObject>> m_mapGameObjects;

// 재정의입니다.
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnHierarchyProperties();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSort(UINT id);
	afx_msg void OnUpdateSort(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
};
