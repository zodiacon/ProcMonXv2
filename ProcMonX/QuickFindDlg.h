#pragma once

#include "resource.h"
#include "Interfaces.h"

class CQuickFindDlg : public CDialogImpl<CQuickFindDlg> {
public:
	enum { IDD = IDD_FIND };

	CQuickFindDlg(IQuickFind* qf) : m_QuickFind(qf) {}

	static const QuickFindOptions& GetSearchOptions();
	static const CString& GetSearchText();

	void OnFinalMessage(HWND) override;

	BEGIN_MSG_MAP(CQuickFindDlg)
		COMMAND_CODE_HANDLER(EN_CHANGE, OnTextChanged)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_FINDNEXT, OnFindNext)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
private:
	void CheckButton(UINT id, bool check);
	bool IsChecked(UINT id) const;

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFindNext(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTextChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	IQuickFind* m_QuickFind;
	inline static QuickFindOptions m_Options;
	inline static CString m_SearchText;
};
