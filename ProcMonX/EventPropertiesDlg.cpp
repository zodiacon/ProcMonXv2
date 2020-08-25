#include "pch.h"
#include "EventPropertiesDlg.h"
#include "EventData.h"
#include "DialogHelper.h"
#include "SortHelper.h"

CEventPropertiesDlg::CEventPropertiesDlg(EventData* data) : m_pData(data) {
}

int CEventPropertiesDlg::InsertItem(PCWSTR name, PCWSTR value) {
	int n = m_List.InsertItem(m_List.GetItemCount(), name);
	m_List.SetItemText(n, 1, value);
	m_List.SetItemData(n, (DWORD_PTR)name);

	return n;
}

LRESULT CEventPropertiesDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_INFOTIP);

	SetIcon(AtlLoadIconImage(IDI_PROPERTIES, 0, 16, 16), FALSE);
	SetIcon(AtlLoadIconImage(IDI_PROPERTIES, 0, 32, 32), TRUE);

	DialogHelper::AddIconToButton(this, IDC_COPY, IDI_COPY);

	DlgResize_Init(true);

	CString text;
	text.Format(L"Event #%u Properties)", m_pData->GetIndex());
	SetWindowText(text);

	m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 120);
	m_List.InsertColumn(1, L"Value", LVCFMT_LEFT, 200);

	auto ts = m_pData->GetTimeStamp();
	text.Format(L".%06u", (ts / 10) % 1000000);

	InsertItem(L"Time Stamp", CTime(*(FILETIME*)&ts).Format(L"%x %X") + text);
	InsertItem(L"Event Name", m_pData->GetEventName().c_str());
	::StringFromGUID2(m_pData->GetProviderId(), text.GetBufferSetLength(64), 64);
	InsertItem(L"Provider Id", text);
	text.Format(L"%d", m_pData->GetEventDescriptor().Opcode);
	InsertItem(L"Opcode", text);

	auto pid = m_pData->GetProcessId();
	if (pid != (DWORD)-1 && pid != 0) {
		text.Format(L"%u (0x%X)", pid, pid);
		InsertItem(L"Process Id", text);
		InsertItem(L"Process Name", m_pData->GetProcessName().c_str());
	}
	auto tid = m_pData->GetThreadId();
	if (tid != (DWORD)-1 && tid != 0) {
		text.Format(L"%u (0x%X)", tid, tid);
		InsertItem(L"Thread Id", text);
	}

	for (auto& prop : m_pData->GetProperties()) {
		if (prop.Name.substr(0, 8) == L"Reserved")
			continue;
		InsertItem(prop.Name.c_str(), m_pData->FormatProperty(prop).c_str());
	}

	return 0;
}

LRESULT CEventPropertiesDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);
	return 0;
}

LRESULT CEventPropertiesDlg::OnCopy(WORD, WORD wID, HWND, BOOL&) {
	return 0;
}

LRESULT CEventPropertiesDlg::OnColumnClick(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLISTVIEW*)hdr;
	if (lv->iSubItem == 0) {
		m_Ascending = !m_Ascending;
		m_List.SortItems([](auto p1, auto p2, auto asc) {
			return SortHelper::SortStrings((PCWSTR)p1, (PCWSTR)p2, (bool)asc) ? -1 : 1;
			}, (LPARAM)m_Ascending);
	}
	return 0;
}
