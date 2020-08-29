#pragma once

class TraceManager;

struct IMainFrame {
	virtual BOOL TrackPopupMenu(HMENU hMenu, HWND hWnd, POINT* pt = nullptr, UINT flags = 0) = 0;
	virtual void ViewDestroyed(void* p) = 0;
	virtual TraceManager& GetTraceManager() = 0;
	virtual CFont& GetMonoFont() = 0;
	virtual BOOL SetPaneText(int index, PCWSTR text) = 0;
	virtual BOOL SetPaneIcon(int index, HICON hIcon) = 0;
	virtual CUpdateUIBase* GetUpdateUI() = 0;
};
