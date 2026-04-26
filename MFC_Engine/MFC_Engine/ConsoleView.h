#pragma once

/////////////////////////////////////////////////////////////////////////////
// CConsoleList 창

class CConsoleList : public CListBox
{
// 생성입니다.
public:
	CConsoleList() noexcept;

// 구현입니다.
public:
	virtual ~CConsoleList();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
};

class CConsoleView : public CDockablePane
{
// 생성입니다.
public:
	CConsoleView() noexcept;

	void UpdateFonts();

// 특성입니다.
protected:
	CMFCTabCtrl	m_wndTabs;

	CConsoleList m_wndOutputBuild;
	CConsoleList m_wndOutputDebug;
	CConsoleList m_wndOutputFind;

protected:
	void FillBuildWindow();
	void FillDebugWindow();
	void FillFindWindow();

	void AdjustHorzScroll(CListBox& wndListBox);

// 구현입니다.
public:
	virtual ~CConsoleView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};
