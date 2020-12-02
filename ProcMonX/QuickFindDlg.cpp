#include "pch.h"
#include "QuickFindDlg.h"
#include "DialogHelper.h"

const QuickFindOptions& CQuickFindDlg::GetSearchOptions() {
	return m_Options;
}

const CString& CQuickFindDlg::GetSearchText() {
	return m_SearchText;
}

void CQuickFindDlg::OnFinalMessage(HWND) {
	m_QuickFind->WindowClosed();
	delete this;
}

void CQuickFindDlg::CheckButton(UINT id, bool check) {
	CheckDlgButton(id, check ? BST_CHECKED : BST_UNCHECKED);
}

bool CQuickFindDlg::IsChecked(UINT id) const {
	return IsDlgButtonChecked(id) == BST_CHECKED;
}

LRESULT CQuickFindDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	DialogHelper::SetDialogIcon(this, IDI_FIND);

	CheckButton(IDC_CASESENSITIVE, m_Options.CaseSensitive);
	CheckButton(IDC_DOWN, m_Options.SearchDown);
	CheckButton(IDC_PROCESSES, m_Options.SearchProcesses);
	CheckButton(IDC_EVENT, m_Options.SearchEvents);
	CheckButton(IDC_DETAILS, m_Options.SearchDetails);

	return 0;
}

LRESULT CQuickFindDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	m_Options.CaseSensitive = IsChecked(IDC_CASESENSITIVE);
	m_Options.SearchDetails = IsChecked(IDC_DETAILS);
	m_Options.SearchProcesses = IsChecked(IDC_PROCESSES);
	m_Options.SearchEvents = IsChecked(IDC_EVENT);
	m_Options.SearchDown = IsChecked(IDC_DOWN);

	DestroyWindow();
	return 0;
}

LRESULT CQuickFindDlg::OnFindNext(WORD, WORD wID, HWND, BOOL&) {
	m_QuickFind->DoFind(m_SearchText, m_Options);

	return 0;
}

LRESULT CQuickFindDlg::OnTextChanged(WORD, WORD wID, HWND, BOOL&) {
	GetDlgItem(IDC_TEXT).GetWindowText(m_SearchText);
	GetDlgItem(IDC_FINDNEXT).EnableWindow(!m_SearchText.IsEmpty());

	return 0;
}
