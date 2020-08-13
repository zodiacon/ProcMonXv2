// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ViewBase.h"
#include "VirtualListView.h"
#include "Interfaces.h"
#include "EventData.h"
#include <shared_mutex>

class CView : 
	public CViewBase<CView>,
	public CVirtualListView<CView>,
	public CCustomDraw<CView> {
public:
	CView(IMainFrame* frame);

	DECLARE_WND_CLASS(NULL)

	void AddEvent(std::shared_ptr<EventData> data);
	void StartMonitoring(bool start);
	CString GetColumnText(HWND, int row, int col) const;

	BOOL PreTranslateMessage(MSG* pMsg);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		CHAIN_MSG_MAP(CVirtualListView<CView>)
		CHAIN_MSG_MAP(CCustomDraw<CView>)
		CHAIN_MSG_MAP(CViewBase<CView>)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	CListViewCtrl m_List;
	std::vector<std::shared_ptr<EventData>> m_Events;
	std::vector<std::shared_ptr<EventData>> m_TempEvents;
	std::mutex m_EventsLock;
	bool m_IsMonitoring{ false };
};
