#include "pch.h"
#include "CallStackDlg.h"
#include "EventData.h"

CCallStackDlg::CCallStackDlg(EventData* data) : m_pData(data) {
	ATLASSERT(data);
}

LRESULT CCallStackDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	m_List.Attach(GetDlgItem(IDC_LIST));

	DlgResize_Init(true);

	auto stack = m_pData->GetStackEventData();
	ATLASSERT(stack);

	CString num;
	for (int i = 1; i <= 192; i++) {
		num.Format(L"Stack%d", i);
		auto prop = stack->GetProperty(num);
		if (prop == nullptr)
			break;

		DWORD64 address;
		if (prop->GetLength() == 4)
			address = prop->GetValue<DWORD>();
		else
			address = prop->GetValue<DWORD64>();
		num.Format(L"0x%p", (PVOID)address);
		m_List.AddString(num);
	}

	return 0;
}

LRESULT CCallStackDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);
	return 0;
}
