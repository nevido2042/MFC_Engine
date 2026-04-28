
// MainFrm.h: CMainFrame 클래스의 인터페이스
//

#pragma once
#include "ProjectView.h"
#include "HierarchyView.h"
#include "ConsoleView.h"
#include "InspectorView.h"
#include "SceneView.h"
#include "GameView.h"
#include "GBufferView.h"
class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame() noexcept;

// 특성입니다.
public:
	CInspectorView* GetInspectorView() { return &m_wndInspectorView; }
	CHierarchyView* GetHierarchyView() { return &m_wndHierarchyView; }
	CSceneView* GetSceneView() { return &m_wndSceneView; }
	CProjectView* GetProjectView() { return &m_wndProjectView; }


// 작업입니다.
public:
	void OnSceneLoaded();

// 재정의입니다.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);

// 구현입니다.
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 컨트롤 모음이 포함된 멤버입니다.
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;
	CProjectView      m_wndProjectView;
	CHierarchyView    m_wndHierarchyView;
	CConsoleView      m_wndConsoleView;
	CInspectorView    m_wndInspectorView;
	CSceneView        m_wndSceneView;
	CGameView         m_wndGameView;
	CGBufferView      m_wndGBufferView;

// 생성된 메시지 맵 함수
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnFileSave();
	afx_msg void OnFileOpen();
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
};


