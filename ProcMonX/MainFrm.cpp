// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "aboutdlg.h"
#include "MainFrm.h"
#include "EventData.h"

const int WINDOW_MENU_POSITION = 6;

CMainFrame::CMainFrame() {
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() {
	UIUpdateToolBar();
	UIUpdateStatusBar();

	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	::SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	::SetUnhandledExceptionFilter(UnhandledExceptionFilter);

	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	CMenuHandle hMenu = GetMenu();
	UIAddMenu(hMenu);
	m_CmdBar.AttachMenu(hMenu);
	m_CmdBar.m_bAlphaImages = true;
	SetMenu(nullptr);

	InitCommandBar();

	CToolBarCtrl tb;
	auto hWndToolBar = tb.Create(m_hWnd, nullptr, nullptr, ATL_SIMPLE_TOOLBAR_PANE_STYLE, 0, ATL_IDW_TOOLBAR);
	InitToolBar(tb);
	m_ToolBar.Attach(hWndToolBar);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, nullptr, TRUE);
	UIAddToolBar(hWndToolBar);

	CReBarCtrl(m_hWndToolBar).LockBands(TRUE);

	CreateSimpleStatusBar();
	m_StatusBar.SubclassWindow(m_hWndStatusBar);
	int widths[] = { 100, 130, 300 };
	m_StatusBar.SetParts(_countof(widths), widths);

	UIAddStatusBar(m_hWndStatusBar);

	m_view.m_bTabCloseButton = false;
	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	CMenuHandle menuMain = m_CmdBar.GetMenu();
	m_view.SetWindowMenu(menuMain.GetSubMenu(WINDOW_MENU_POSITION));

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 4, 4);
	UINT icons[] = { IDI_EVENT, IDI_PLAY, IDI_PAUSE };
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_view.SetImageList(images);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	m_RunIcon.LoadIconWithScaleDown(IDI_PLAY, 20, 20);
	m_StopIcon.LoadIconWithScaleDown(IDI_STOP, 20, 20);
	m_PauseIcon.LoadIconWithScaleDown(IDI_PAUSE, 20, 20);
	SetPaneIcon(1, m_StopIcon);

	UIEnable(ID_MONITOR_STOP, FALSE);
	UIEnable(ID_MONITOR_PAUSE, FALSE);

	m_MonoFont.CreatePointFont(100, L"Consolas");

	PostMessage(WM_COMMAND, ID_FILE_NEW);

	return 0;
}

LRESULT CMainFrame::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1 && m_pMonitorView) {
		KillTimer(1);
		MEMORYSTATUSEX ms = { sizeof(ms) };
		::GlobalMemoryStatusEx(&ms);
		if (ms.dwMemoryLoad > 94 && m_pMonitorView) {
			if (AtlMessageBox(*this, L"Physical memory is low. Continue monitoring?", 
				IDS_TITLE, MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING | MB_SETFOREGROUND | MB_SYSTEMMODAL) == IDCANCEL) {
				PostMessage(WM_COMMAND, ID_MONITOR_STOP);
			}
		}
	}

	return 0;
}

LRESULT CMainFrame::OnClose(UINT, WPARAM, LPARAM, BOOL&) {
	if (m_tm.IsRunning())
		m_tm.Stop();
	
	return DefWindowProc();
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto pView = new CView(this);
	pView->Create(m_view, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
	m_view.AddPage(pView->m_hWnd, _T("Events"), 0, pView);
	m_pCurrentView = pView;

	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnWindowClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nActivePage = m_view.GetActivePage();
	if (nActivePage != -1) {
		if (m_pMonitorView && m_pMonitorView->m_hWnd == m_view.GetPageHWND(nActivePage)) {
			AtlMessageBox(*this, L"Monitoring in progress. Stop monitoring before closing this tab", IDS_TITLE, MB_ICONWARNING);
			return 0;
		}
		m_view.RemovePage(nActivePage);
	}

	if (m_view.GetPageCount() == 1) {
		::MessageBeep((UINT)-1);
		return 0;
	}

	return 0;
}

LRESULT CMainFrame::OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	m_view.RemoveAllPages();

	return 0;
}

LRESULT CMainFrame::OnWindowActivate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int nPage = wID - ID_WINDOW_TABFIRST;
	m_view.SetActivePage(nPage);

	return 0;
}

LRESULT CMainFrame::OnAlwaysOnTop(WORD, WORD, HWND, BOOL&) {
	return LRESULT();
}

LRESULT CMainFrame::OnMonitorStart(WORD, WORD, HWND, BOOL&) {
	m_pCurrentView->StartMonitoring(m_tm, true);
	m_view.SetPageImage(m_view.GetActivePage(), 1);
	m_pMonitorView = m_pCurrentView;
	m_tm.Start([&](auto data) {
		m_pMonitorView->AddEvent(data);
		});

	UIEnable(ID_MONITOR_STOP, TRUE);
	UIEnable(ID_MONITOR_START, FALSE);
	UIEnable(ID_MONITOR_PAUSE, TRUE);

	SetTimer(1, 5000, nullptr);
	SetPaneIcon(1, m_RunIcon);
	return 0;
}

LRESULT CMainFrame::OnMonitorStop(WORD, WORD, HWND, BOOL&) {
	KillTimer(1);
	m_tm.Stop();
	m_tm.Pause(false);

	m_view.SetPageImage(m_view.PageIndexFromHwnd(m_pMonitorView->m_hWnd), 0);
	m_pMonitorView->StartMonitoring(m_tm, false);
	m_pMonitorView = nullptr;
	SetPaneIcon(1, m_StopIcon);

	UIEnable(ID_MONITOR_STOP, FALSE);
	UIEnable(ID_MONITOR_START, TRUE);
	UIEnable(ID_MONITOR_PAUSE, FALSE);
	UISetCheck(ID_MONITOR_PAUSE, FALSE);

	return 0;
}

LRESULT CMainFrame::OnMonitorPause(WORD, WORD, HWND, BOOL&) {
	m_tm.Pause(!m_tm.IsPaused());
	UISetCheck(ID_MONITOR_PAUSE, m_tm.IsPaused());
	int image = 2;
	HICON hIcon = m_PauseIcon;
	if (!m_tm.IsPaused()) {
		image = m_tm.IsRunning() ? 1 : 0;
		hIcon = m_tm.IsRunning() ? m_RunIcon : m_StopIcon;
	}
	m_view.SetPageImage(m_view.PageIndexFromHwnd(m_pMonitorView->m_hWnd), image);
	SetPaneIcon(1, hIcon);

	return 0;
}

LRESULT CMainFrame::OnForwardToActiveTab(WORD, WORD wID, HWND, BOOL&) {
	auto page = m_view.GetActivePage();
	if (page >= 0) {
		auto hWnd = m_view.GetPageHWND(page);
		::SendMessage(hWnd, WM_COMMAND, wID, 0);
	}
	return 0;
}

LRESULT CMainFrame::OnTabActivated(int, LPNMHDR hdr, BOOL&) {
	if (m_pCurrentView)
		m_pCurrentView->Activate(false);
	int page = (int)hdr->idFrom;
	if (page < 0) {
		m_pCurrentView = nullptr;
	}
	else {
		m_pCurrentView = (CView*)m_view.GetPageData(page);
		m_pCurrentView->Activate(true);
	}

	return 0;
}

BOOL CMainFrame::TrackPopupMenu(HMENU hMenu, HWND hWnd, POINT* pt, UINT flags) {
	POINT cursorPos;
	if (pt == nullptr) {
		::GetCursorPos(&cursorPos);
		pt = &cursorPos;
	}
	return m_CmdBar.TrackPopupMenu(hMenu, flags, pt->x, pt->y);
}

HFONT CMainFrame::GetMonoFont() {
	return m_MonoFont;
}

void CMainFrame::ViewDestroyed(void* view) {
}

TraceManager& CMainFrame::GetTraceManager() {
	return m_tm;
}

BOOL CMainFrame::SetPaneText(int index, PCWSTR text) {
	return m_StatusBar.SetText(index, text);
}

BOOL CMainFrame::SetPaneIcon(int index, HICON hIcon) {
	return m_StatusBar.SetIcon(index, hIcon);
}

CUpdateUIBase* CMainFrame::GetUpdateUI() {
	return this;
}

void CMainFrame::InitCommandBar() {
	struct {
		UINT id, icon;
	} cmds[] = {
		{ ID_MONITOR_START, IDI_PLAY },
		{ ID_MONITOR_STOP, IDI_STOP },
		{ ID_MONITOR_CLEAR, IDI_CANCEL },
		{ ID_MONITOR_CONFIGUREEVENTS, IDI_TOOLS },
		{ ID_MONITOR_FILTERS, IDI_FILTER },
		{ ID_EVENT_CALLSTACK, IDI_STACK },
		{ ID_EVENT_PROPERTIES, IDI_PROPERTIES },
		{ ID_FILE_SAVE, IDI_SAVE },
		{ ID_FILE_SAVE_AS, IDI_SAVE_AS },
		{ ID_EDIT_COPY, IDI_COPY },
		{ ID_MONITOR_PAUSE, IDI_PAUSE },
	};
	for (auto& cmd : cmds)
		m_CmdBar.AddIcon(AtlLoadIcon(cmd.icon), cmd.id);
}

void CMainFrame::InitToolBar(CToolBarCtrl& tb, int size) {
	CImageList tbImages;
	tbImages.Create(size, size, ILC_COLOR32, 8, 4);
	tb.SetImageList(tbImages);

	struct {
		UINT id;
		int image;
		int style = BTNS_BUTTON;
	} buttons[] = {
		{ ID_MONITOR_START, IDI_PLAY },
		{ ID_MONITOR_PAUSE, IDI_PAUSE },
		{ ID_MONITOR_STOP, IDI_STOP },
		{ 0 },
		{ ID_MONITOR_CLEAR, IDI_CANCEL },
		{ 0 },
		{ ID_MONITOR_CONFIGUREEVENTS, IDI_TOOLS },
		{ ID_MONITOR_FILTERS, IDI_FILTER },
		{ 0 },
		{ ID_EVENT_CALLSTACK, IDI_STACK },
		{ ID_EVENT_PROPERTIES, IDI_PROPERTIES },

	};
	for (auto& b : buttons) {
		if (b.id == 0)
			tb.AddSeparator(0);
		else {
			int image = tbImages.AddIcon(AtlLoadIconImage(b.image, 0, size, size));
			tb.AddButton(b.id, b.style, TBSTATE_ENABLED, image, nullptr, 0);
		}
	}
}

LONG __stdcall CMainFrame::UnhandledExceptionFilter(EXCEPTION_POINTERS* ei) {
	if (EXCEPTION_ACCESS_VIOLATION == ei->ExceptionRecord->ExceptionCode) {
		// probably the network related thread failing during symbol loading when terminated abruptly
		::ExitThread(0);
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	
	return EXCEPTION_EXECUTE_HANDLER;
}

