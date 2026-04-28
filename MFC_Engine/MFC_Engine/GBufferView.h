#pragma once
#include <afxdockablepane.h>
#include <memory>

class CGBufferView : public CDockablePane
{
public:
    CGBufferView() noexcept;
    virtual ~CGBufferView();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

    DECLARE_MESSAGE_MAP()

public:
    void SetEngine(class CGraphicsEngine* pEngine);

private:
    class CGraphicsEngine* m_pEngine;
};
