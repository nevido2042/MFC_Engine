#include "pch.h"
#include "framework.h"
#include "GameView.h"
#include "MainFrm.h"
#include "Resource.h"
#include "MFC_Engine.h"
#include "SceneManager.h"
#include "RenderPassFactory.h"
#include "RenderPass.h"
#include "SwapChain.h"
#include "GBuffer.h"
#include "ImGuiManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGameView::CGameView() noexcept
{
}

CGameView::~CGameView()
{
}

BEGIN_MESSAGE_MAP(CGameView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

int CGameView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	/*
	// Graphics Engine 초기화
	m_pEngine = std::make_unique<CGraphicsEngine>();

	CRect rect;
	GetClientRect(rect);

	CDevice* pDevice = theApp.GetDevice();
	m_pEngine->Initialize(pDevice, rect.Width(), rect.Height());

	// 컴포넌트 주입
	auto pSwapChain = std::make_unique<CSwapChain>();
	pSwapChain->Initialize(pDevice, GetSafeHwnd(), rect.Width(), rect.Height());
	m_pEngine->SetSwapChain(std::move(pSwapChain));

	auto pGBuffer = std::make_unique<CGBuffer>();
	pGBuffer->Initialize(pDevice, rect.Width(), rect.Height());
	m_pEngine->SetGBuffer(std::move(pGBuffer));

	// GameView는 보통 ImGui가 필요 없을 수 있지만, 필요시 주입
	auto pImGuiManager = std::make_unique<CImGuiManager>();
	pImGuiManager->Initialize(GetSafeHwnd(), pDevice->GetDevice(), pDevice->GetCommandQueue(), 2);
	m_pEngine->SetImGuiManager(std::move(pImGuiManager));

	// 렌더 패스 구성
	std::unique_ptr<IRenderPassFactory> pPassFactory =
		std::make_unique<CDX12RenderPassFactory>(pDevice->GetDevice());

	m_pEngine->RegisterRenderPass(pPassFactory->CreatePass("Geometry"));
	m_pEngine->RegisterRenderPass(pPassFactory->CreatePass("Lighting"));

	// 렌더링 스레드 시작
	m_bIsRunning = true;
	m_renderThread = std::thread(&CGameView::RenderLoop, this);
	*/

	return 0;
}

void CGameView::OnDestroy()
{
	/*
	m_bIsRunning = false;
	if (m_renderThread.joinable())
		m_renderThread.join();
	*/

	CDockablePane::OnDestroy();
}

void CGameView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	if (m_pEngine && cx > 0 && cy > 0)
	{
		m_pEngine->Resize(cx, cy);
	}
}

void CGameView::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(rect);

	dc.FillSolidRect(rect, RGB(32, 32, 32)); // 더 어두운 배경
	
	dc.SetTextColor(RGB(200, 200, 200));
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(_T("Game View (Inactive)"), rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CGameView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
}

void CGameView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown(nFlags, point);
}

void CGameView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonUp(nFlags, point);
}

void CGameView::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);
}

void CGameView::RenderLoop()
{
	while (m_bIsRunning)
	{
		if (m_pEngine)
		{
			auto pScene = CSceneManager::GetInstance().GetActiveScene();
			if (pScene)
			{
				m_pEngine->Render(pScene, nullptr);
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
