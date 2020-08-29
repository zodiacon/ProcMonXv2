#include "pch.h"
#include "CallStackDlg.h"
#include "EventData.h"
#include "SymbolManager.h"
#include "ClipboardHelper.h"
#include "DialogHelper.h"

CCallStackDlg::CCallStackDlg(EventData* data) : m_pData(data) {
	ATLASSERT(data);
}

DWORD CCallStackDlg::LoadSymbolsThread() {
	auto stack = m_pData->GetStackEventData();
	auto& symbols = SymbolManager::Get();
	auto pid = stack->GetProperty(L"StackProcess")->GetValue<DWORD>();

	CString num;
	for (int i = 1; i <= 192; i++) {
		if (m_ExitSymbolThread)
			break;

		num.Format(L"Stack%d", i);
		auto prop = stack->GetProperty(num);
		if (prop == nullptr)
			break;

		DWORD64 address;
		if (prop->GetLength() == 4)
			address = prop->GetValue<DWORD>();
		else
			address = prop->GetValue<DWORD64>();
		DWORD64 offset = 0;
		auto symbol = symbols.GetSymbolFromAddress(pid, address, &offset);
		if (symbol) {
			auto sym = symbol->GetSymbolInfo();
			CStringA text;
			text.Format("%s!%s+0x%X", symbol->ModuleInfo.ModuleName, sym->Name, (DWORD)offset);
			SendMessage(WM_USER + 100, i, (LPARAM)(PCSTR)text);
		}
	}
	SendMessage(WM_USER + 100, 0, 0);

	return 0;
}

LRESULT CCallStackDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	m_List.Attach(GetDlgItem(IDC_LIST));

	DialogHelper::SetDialogIcon(this, IDI_STACK);
	DialogHelper::AddIconToButton(this, IDC_COPY, IDI_COPY);

	CString num;
	num.Format(L"Call Stack (Event #%u)", m_pData->GetIndex());
	SetWindowText(num);

	DlgResize_Init(true);

	auto stack = m_pData->GetStackEventData();
	ATLASSERT(stack);
	if (stack == nullptr) {
		EndDialog(IDCANCEL);
		return 0;
	}

	auto& symbols = SymbolManager::Get();
	auto pid = stack->GetProperty(L"StackProcess")->GetValue<DWORD>();
	auto tid = stack->GetProperty(L"StackThread")->GetValue<DWORD>();
	SetDlgItemInt(IDC_PROCESS, pid);
	SetDlgItemInt(IDC_THREAD, tid);

	m_List.InsertColumn(0, L"#", LVCFMT_LEFT, 45);
	m_List.InsertColumn(1, L"Address", LVCFMT_RIGHT, 140);
	m_List.InsertColumn(2, L"Symbol", LVCFMT_LEFT, 280);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 2, 0);
	images.AddIcon(AtlLoadIconImage(IDI_K, 0, 16, 16));
	images.AddIcon(AtlLoadIconImage(IDI_U, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	SetDlgItemText(IDC_MESSAGE, L"Loading symbols...");

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
		num.Format(L"%2d", i);
		int n = m_List.InsertItem(i - 1, num, (int64_t)address < 0 ? 0 : 1);
		num.Format(L"0x%p", (PVOID)address);
		m_List.SetItemText(n, 1, num);
	}

	// create thread to load symbols
	m_hThread.reset(::CreateThread(nullptr, 0, [](auto param) {
		return ((CCallStackDlg*)param)->LoadSymbolsThread();
		}, this, 0, nullptr));
	::SetThreadPriority(m_hThread.get(), THREAD_PRIORITY_LOWEST);

	return 0;
}

LRESULT CCallStackDlg::OnSymbolLoaded(UINT, WPARAM wp, LPARAM lp, BOOL&) {
	if (lp)
		m_List.SetItemText((int)wp - 1, 2, CString((PCSTR)lp));
	else
		SetDlgItemText(IDC_MESSAGE, L"");

	return 0;
}

LRESULT CCallStackDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	m_ExitSymbolThread = true;
	if (WAIT_TIMEOUT == ::WaitForSingleObject(m_hThread.get(), 1500)) {
		::TerminateThread(m_hThread.get(), 1);
	}
	EndDialog(wID);
	return 0;
}

LRESULT CCallStackDlg::OnCopy(WORD, WORD wID, HWND, BOOL&) {
	CString text, item;
	for (int i = 0; i < m_List.GetItemCount(); i++) {
		for (int c = 0; c < 3; c++) {
			m_List.GetItemText(i, c, item);
			text += item += L",";
		}
		text = text.Left(text.GetLength() - 1) + L"\n";
	}
	ClipboardHelper::CopyText(*this, text);
	return 0;
}
