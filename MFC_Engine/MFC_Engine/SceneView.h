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
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CGraphicsEngine* GetEngine() { return m_pEngine.get(); }
	void SetSelectedGameObject(std::shared_ptr<class CGameObject> pObj) 
	{ 
		std::lock_guard<std::mutex> lock(m_selectionMutex);
		m_pSelectedObj = pObj; 
	}
	std::shared_ptr<class CGameObject> GetSelectedGameObject() const 
	{ 
		std::lock_guard<std::mutex> lock(m_selectionMutex);
		return m_pSelectedObj; 
	}

private:
	mutable std::mutex m_selectionMutex;
	void RenderLoop();
	void ProcessInput(float deltaTime);

private:
	std::unique_ptr<CGraphicsEngine> m_pEngine;
	std::thread m_renderThread;
	std::atomic<bool> m_bIsRunning;

	// --- 선택 및 기즈모 ---
	std::shared_ptr<class CGameObject> m_pSelectedObj;
	std::unique_ptr<class CGizmo> m_pGizmo;

	bool m_bRButtonDown;
	bool m_bLButtonDown;
	int m_gizmoAxis; // 0: none, 1: X, 2: Y, 3: Z
	CPoint m_lastMousePos;
	bool m_keys[256];
};
