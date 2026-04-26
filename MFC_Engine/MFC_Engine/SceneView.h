#pragma once
#include "GraphicsEngine.h"
#include <thread>
#include <atomic>
#include <chrono>

class CSceneView : public CDockablePane
{
public:
	CSceneView() noexcept;
	virtual ~CSceneView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

private:
	void RenderLoop();
	void ProcessInput(float deltaTime);

private:
	std::unique_ptr<CGraphicsEngine> m_pEngine;
	std::thread m_renderThread;
	std::atomic<bool> m_bIsRunning;

	// --- 입력 관련 ---
	bool m_bRButtonDown;
	CPoint m_lastMousePos;
	bool m_keys[256];
};
