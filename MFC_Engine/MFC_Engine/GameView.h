#pragma once
#include "GraphicsEngine.h"
#include <thread>
#include <atomic>

class CGameView : public CDockablePane
{
public:
	CGameView() noexcept;
	virtual ~CGameView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

public:
	CGraphicsEngine* GetEngine() { return m_pEngine.get(); }

private:
	void RenderLoop();

private:
	std::unique_ptr<CGraphicsEngine> m_pEngine;
	std::thread m_renderThread;
	std::atomic<bool> m_bIsRunning;
};
