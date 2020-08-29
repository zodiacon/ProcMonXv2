#pragma once

#include "resource.h"
#include "FilterBase.h"
#include "FilterConfiguration.h"

class CFiltersDlg : 
	public CDialogImpl<CFiltersDlg> {
public:
	enum { IDD = IDD_FILTERS };

	CFiltersDlg(FilterConfiguration& fc);

	BEGIN_MSG_MAP(CFiltersDlg)
		COMMAND_CODE_HANDLER(CBN_EDITCHANGE, OnValueChanged)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged) 
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_UP, OnMoveUp)
		COMMAND_ID_HANDLER(IDC_DOWN, OnMoveDown)
		COMMAND_ID_HANDLER(IDC_DELETE, OnDelete)
		COMMAND_ID_HANDLER(IDC_ADD, OnAdd)
		COMMAND_ID_HANDLER(IDC_SAVE, OnSave)
		COMMAND_ID_HANDLER(IDC_LOAD, OnLoad)
		COMMAND_ID_HANDLER(IDC_EDIT, OnEdit)
	END_MSG_MAP()

private:
	static CString CompareTypeToString(CompareType compare);
	static CString GetComboText(CComboBox& cb);
	static bool SwapItems(CListViewCtrl& lv, int i1, int i2);

	void InitList(const FilterConfiguration& config);
	void UpdateUI();
	void UpdateConfig(FilterConfiguration& config);

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAdd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSave(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLoad(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEdit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMoveUp(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMoveDown(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDelete(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnValueChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

	FilterConfiguration& m_Config;
	CListViewCtrl m_List;
	CComboBox m_FilterNames;
	CComboBox m_CompareTypes;
	CComboBox m_Text;
	CComboBox m_IncExc;
};
