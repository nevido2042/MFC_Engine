// MFC_EngineView.h: CMFCEngineView 클래스의 인터페이스
//

#pragma once

class CGraphicsEngine;

class CMFCEngineView : public CView
{
protected: // serialization에서만 만들어집니다.
	CMFCEngineView() noexcept;
	DECLARE_DYNCREATE(CMFCEngineView)

// 특성입니다.
public:
	CMFCEngineDoc* GetDocument() const;

// 작업입니다.
public:

private:
	std::unique_ptr<CGraphicsEngine> m_graphicsEngine;

// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현입니다.
public:
	virtual ~CMFCEngineView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
};

#ifndef _DEBUG  // MFC_EngineView.cpp의 디버그 버전
inline CMFCEngineDoc* CMFCEngineView::GetDocument() const
   { return reinterpret_cast<CMFCEngineDoc*>(m_pDocument); }
#endif

