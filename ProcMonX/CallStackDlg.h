#pragma once

#include "resource.h"
#include <wil\resource.h>
#include <atomic>

class EventData;
class SymbolsHandler;

class CCallStackDlg : 
	public CDialogImpl<CCallStackDlg>,
	public CDialogResize<CCallStackDlg> {
public:
	enum { IDD = IDD_STACK };

	CCallStackDlg(EventData* data);

	BEGIN_DLGRESIZE_MAP(CCallStackDlg)
		BEGIN_DLGRESIZE_GROUP()
			DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_Y | DLSZ_SIZE_X)
		END_DLGRESIZE_GROUP()
	END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_USER + 100, OnSymbolLoaded)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_COPY, OnCopy)
		CHAIN_MSG_MAP(CDialogResize<CCallStackDlg>)
	END_MSG_MAP()

private:
	DWORD LoadSymbolsThread();

	// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSymbolLoaded(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	std::atomic<bool> m_ExitSymbolThread{ false };
	EventData* m_pData;
	CListViewCtrl m_List;
	wil::unique_handle m_hThread;
};

