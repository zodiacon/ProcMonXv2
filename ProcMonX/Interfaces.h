#pragma once

class TraceManager;

struct QuickFindOptions {
	bool CaseSensitive : 1;
	bool SearchProcesses : 1 { true};
	bool SearchEvents : 1 { true };
	bool SearchDetails : 1;
	bool SearchDown : 1 { true };
	bool FindNext : 1 { true };
};

struct IMainFrame {
	virtual BOOL TrackPopupMenu(HMENU hMenu, HWND hWnd, POINT* pt = nullptr, UINT flags = 0) = 0;
	virtual void ViewDestroyed(void* p) = 0;
	virtual TraceManager& GetTraceManager() = 0;
	virtual HFONT GetMonoFont() = 0;
	virtual BOOL SetPaneText(int index, PCWSTR text) = 0;
	virtual BOOL SetPaneIcon(int index, HICON hIcon) = 0;
	virtual CUpdateUIBase* GetUpdateUI() = 0;
};

struct IQuickFind {
	virtual void DoFind(PCWSTR text, const QuickFindOptions& options) = 0;
	virtual void WindowClosed() = 0;
};
