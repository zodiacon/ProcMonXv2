#pragma once

#include "resource.h"

class EventsConfiguration;

class CEventsDlg :
	public CDialogImpl<CEventsDlg>,
	public CDialogResize<CEventsDlg> {
public:
	enum { IDD = IDD_EVENTS };

	CEventsDlg(EventsConfiguration& config);

	BEGIN_MSG_MAP(CEventsDlg)
		NOTIFY_CODE_HANDLER(TVN_ITEMCHANGED, OnTreeItemChanged)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		COMMAND_HANDLER(IDC_BASIC, BN_CLICKED, OnBasicView)
		COMMAND_HANDLER(IDC_ADVANCED, BN_CLICKED, OnAdvancedView)
		COMMAND_ID_HANDLER(IDC_SAVE, OnSave)
		COMMAND_ID_HANDLER(IDC_LOAD, OnLoad)
		COMMAND_ID_HANDLER(IDC_EXPAND, OnExpandAll)
		COMMAND_ID_HANDLER(IDC_COLLAPSE, OnCollapseAll)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CDialogResize<CEventsDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEventsDlg)
		DLGRESIZE_CONTROL(IDC_TREE, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	void BuildEventsTree(const EventsConfiguration& config);
	void CheckTreeChildren(HTREEITEM hParent, bool check);
	bool BuildConfigFromTree(EventsConfiguration& config);
	void SwitchToAdvancedView(bool advanced);

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD id, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTreeItemChanged(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/);
	LRESULT OnSave(WORD /*wNotifyCode*/, WORD id, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLoad(WORD /*wNotifyCode*/, WORD id, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExpandAll(WORD /*wNotifyCode*/, WORD id, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCollapseAll(WORD /*wNotifyCode*/, WORD id, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBasicView(WORD /*wNotifyCode*/, WORD id, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAdvancedView(WORD /*wNotifyCode*/, WORD id, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	EventsConfiguration& m_Config;
	EventsConfiguration m_TempConfig;
	CTreeViewCtrlEx m_Tree;
	int m_Recurse{ 0 };
	bool m_Init{ true };
	inline static bool m_Advanced{ false };
};

