#pragma once
#include "GraphicsEngine.h"
#include <thread>
#include <atomic>

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

	DECLARE_MESSAGE_MAP()

private:
	void RenderLoop();

private:
	std::unique_ptr<CGraphicsEngine> m_pEngine;
	std::thread m_renderThread;
	std::atomic<bool> m_bIsRunning;
};
