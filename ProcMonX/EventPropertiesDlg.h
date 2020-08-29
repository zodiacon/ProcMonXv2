#pragma once

#include "resource.h"

class EventData;

class CEventPropertiesDlg : 
	public CDialogImpl<CEventPropertiesDlg>,
	public CDialogResize<CEventPropertiesDlg> {
public:
	enum { IDD = IDD_EVENT_PROPS };

	CEventPropertiesDlg(EventData* data);

	BEGIN_DLGRESIZE_MAP(CCallStackDlg)
		DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_Y | DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_MSG_MAP(CEventPropertiesDlg)
		NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_COPY, OnCopy)
		COMMAND_ID_HANDLER(IDC_STACK, OnCallStack)
		CHAIN_MSG_MAP(CDialogResize<CEventPropertiesDlg>)
	END_MSG_MAP()

private:
	int InsertItem(PCWSTR name, PCWSTR value);

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnColumnClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnCallStack(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	EventData* m_pData;
	CListViewCtrl m_List;
	bool m_Ascending{ false };
};

