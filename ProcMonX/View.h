// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ViewBase.h"
#include "VirtualListView.h"
#include "Interfaces.h"
#include "EventData.h"
#include <shared_mutex>
#include "EventConfiguration.h"
#include "SymbolsHandler.h"

class CView : 
	public CViewBase<CView>,
	public CVirtualListView<CView>,
	public CCustomDraw<CView> {
public:
	CView(IMainFrame* frame);

	DECLARE_WND_CLASS(NULL)

	void AddEvent(std::shared_ptr<EventData> data);
	void StartMonitoring(TraceManager& tm, bool start);
	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(int row) const;
	PCWSTR GetColumnTextPointer(HWND, int row, int col) const;
	bool OnRightClickList(int index, POINT& pt);

	std::wstring ProcessSpecialEvent(EventData* data);

	bool IsSortable(int col) const;
	void DoSort(const SortInfo* si);

	BOOL PreTranslateMessage(MSG* pMsg);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CView)
//		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		COMMAND_ID_HANDLER(ID_EVENT_CALLSTACK, OnCallStack)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_ID_HANDLER(ID_MONITOR_CONFIGUREEVENTS, OnConfigureEvents)
		COMMAND_ID_HANDLER(ID_MONITOR_CLEAR, OnClear)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CVirtualListView<CView>)
		CHAIN_MSG_MAP(CCustomDraw<CView>)
		CHAIN_MSG_MAP(CViewBase<CView>)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnForwardMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCallStack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnConfigureEvents(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	CListViewCtrl m_List;
	inline static CImageListManaged s_Images;
	std::unordered_map<std::wstring, int> s_IconsMap;
	std::vector<std::shared_ptr<EventData>> m_Events;
	std::vector<std::shared_ptr<EventData>> m_TempEvents;
	std::mutex m_EventsLock;
	EventsConfig m_EventsConfig;
	bool m_IsMonitoring{ false };
	bool m_IsDraining{ false };
};
