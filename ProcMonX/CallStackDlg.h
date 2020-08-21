#pragma once

#include "resource.h"

class EventData;

class CCallStackDlg : 
	public CDialogImpl<CCallStackDlg>,
	public CDialogResize<CCallStackDlg> {
public:
	enum { IDD = IDD_STACK };

	CCallStackDlg(EventData* data);

	BEGIN_DLGRESIZE_MAP(CCallStackDlg)
		BEGIN_DLGRESIZE_GROUP()
			DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_Y | DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y | DLSZ_CENTER_X)
		END_DLGRESIZE_GROUP()
	END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		CHAIN_MSG_MAP(CDialogResize<CCallStackDlg>)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	EventData* m_pData;
	CListBox m_List;
};

