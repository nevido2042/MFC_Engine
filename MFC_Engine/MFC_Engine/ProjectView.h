#pragma once

#include "ViewTree.h"

class CProjectViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CProjectView : public CDockablePane
{
public:
	CProjectView() noexcept;
	virtual ~CProjectView();

	void AdjustLayout();
	void OnChangeVisualStyle();
	void FillProjectView();

protected:
	CViewTree m_wndProjectView;
	CImageList m_ProjectViewImages;
	CProjectViewToolBar m_wndToolBar;

	void PopulateDirectoryTree(const CString& strDirPath, HTREEITEM hParent);

	CString m_strAssetsRoot;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTreeDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};
