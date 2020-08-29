// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "View.h"
#include "TraceManager.h"
#include "SortHelper.h"
#include "CallStackDlg.h"
#include "EventsDlg.h"
#include "EventPropertiesDlg.h"
#include "ProcessIdFilter.h"
#include "FiltersDlg.h"
#include "FilterFactory.h"

CView::CView(IMainFrame* frame) : CViewBase(frame) {
}

void CView::Activate(bool active) {
	m_IsActive = active;
	if (active) {
		UpdateEventStatus();
		UpdateUI();
	}
}

void CView::AddEvent(std::shared_ptr<EventData> data) {
	if (data->GetEventName().empty())
		return;

	//data->SetDetails(details);
	{
		std::lock_guard lock(m_EventsLock);
		m_TempEvents.push_back(data);
	}
}

void CView::StartMonitoring(TraceManager& tm, bool start) {
	if (start) {
		std::vector<KernelEventTypes> types;
		std::vector<std::wstring> categories;
		for (auto& cat : m_EventsConfig.GetCategories()) {
			auto c = KernelEventCategory::GetCategory(cat.Name.c_str());
			ATLASSERT(c);
			types.push_back(c->EnableFlag);
			categories.push_back(c->Name);
		}
		std::initializer_list<KernelEventTypes> events(types.data(), types.data() + types.size());
		tm.SetKernelEventTypes(events);
		tm.SetKernelEventStacks(std::initializer_list<std::wstring>(categories.data(), categories.data() + categories.size()));
		auto selfFilter = std::make_shared<ProcessIdFilter>(::GetCurrentProcessId(), CompareType::Equals, FilterAction::Exclude);
		tm.RemoveAllFilters();
		tm.AddFilter(selfFilter);
	}
	else {
		m_IsDraining = true;
	}
	m_IsMonitoring = start;
}

CString CView::GetColumnText(HWND, int row, int col) const {
	auto item = m_Events[row].get();
	CString text;

	switch (col) {
		case 0:
			text.Format(L"%7u", item->GetIndex());
			if (item->GetStackEventData())
				text += L" *";
			break;

		case 1:
		{
			auto ts = item->GetTimeStamp();
			text.Format(L".%06u", (ts / 10) % 1000000);
			return CTime(*(FILETIME*)&ts).Format(L"%x %X") + text;
		}
		case 3:
		{
			auto pid = item->GetProcessId();
			if (pid != (DWORD)-1)
				text.Format(L"%u (0x%X)", pid, pid);
			break;
		}
		case 5:
		{
			auto tid = item->GetThreadId();
			if (tid != (DWORD)-1 && tid != 0)
				text.Format(L"%u (0x%X)", tid, tid);
			break;
		}
		case 6: text.Format(L"%d", (int)item->GetEventDescriptor().Opcode);
		case 7: 
			::StringFromGUID2(item->GetProviderId(), text.GetBufferSetLength(64), 64);
			break;

		case 8:
			return GetEventDetails(item).c_str();
	}

	return text;
}

int CView::GetRowImage(int row) const {
	auto& evt = m_Events[row];
	if (auto it = s_IconsMap.find(evt->GetEventName()); it != s_IconsMap.end())
		return it->second;

	auto pos = evt->GetEventName().find(L'/');
	if (pos != std::wstring::npos) {
		if (auto it = s_IconsMap.find(evt->GetEventName().substr(0, pos)); it != s_IconsMap.end())
			return it->second;
	}
	return 0;
}

PCWSTR CView::GetColumnTextPointer(HWND, int row, int col) const {
	auto item = m_Events[row].get();
	switch (col) {
		case 2:return item->GetEventName().c_str();
		case 4: return item->GetProcessName().c_str();
//		case 8: return item->GetDetails().c_str();
	}
	return nullptr;
}

bool CView::OnRightClickList(int index, int col, POINT& pt) {
	if (index >= 0) {
		CMenu menu;
		menu.LoadMenuW(IDR_CONTEXT);
		GetFrame()->TrackPopupMenu(menu.GetSubMenu(0), *this, &pt);
		return true;
	}
	return false;
}

bool CView::OnDoubleClickList(int row, int col, POINT& pt) {
	SendMessage(WM_COMMAND, ID_EVENT_PROPERTIES);
	return true;
}

std::wstring CView::ProcessSpecialEvent(EventData* data) const {
	std::wstring details;
	CString text;
	auto& name = data->GetEventName();
	if (name == L"Process/Start") {
		text.Format(L"PID: %u; Image: %s; Command Line: %s",
			data->GetProperty(L"ProcessId")->GetValue<DWORD>(),
			CString(data->GetProperty(L"ImageFileName")->GetAnsiString()),
			data->GetProperty(L"CommandLine")->GetUnicodeString());
		details = std::move(text);
	}
	return details;
}

std::wstring CView::GetEventDetails(EventData* data) const {
	auto details = ProcessSpecialEvent(data);
	if (details.empty()) {
		for (auto& prop : data->GetProperties()) {
			if (prop.Name.substr(0, 8) != L"Reserved" && prop.Name.substr(0, 4) != L"TTID") {
				auto value = data->FormatProperty(prop);
				if (!value.empty()) {
					if (value.size() > 102)
						value = value.substr(0, 100) + L"...";
					details += prop.Name + L": " + value + L"; ";
				}
			}
		}
	}
	return details;
}

void CView::UpdateEventStatus() {
	CString text;
	text.Format(L"Events: %u", (uint32_t)m_Events.size());
	GetFrame()->SetPaneText(2, text);
}

bool CView::IsSortable(int col) const {
	return !m_IsMonitoring && col != 8;
}

void CView::DoSort(const SortInfo* si) {
	std::sort(m_Events.begin(), m_Events.end(), [&](auto& i1, auto& i2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::SortNumbers(i1->GetIndex(), i2->GetIndex(), si->SortAscending);
			case 1: return SortHelper::SortNumbers(i1->GetTimeStamp(), i2->GetTimeStamp(), si->SortAscending);
			case 2: return SortHelper::SortStrings(i1->GetEventName(), i2->GetEventName(), si->SortAscending);
			case 3: return SortHelper::SortNumbers(i1->GetProcessId(), i2->GetProcessId(), si->SortAscending);
			case 4: return SortHelper::SortStrings(i1->GetProcessName(), i2->GetProcessName(), si->SortAscending);
			case 5: return SortHelper::SortNumbers(i1->GetThreadId(), i2->GetThreadId(), si->SortAscending);
			case 6: return SortHelper::SortNumbers(i1->GetEventDescriptor().Opcode, i2->GetEventDescriptor().Opcode, si->SortAscending);
		}
		return false;
		});
}

BOOL CView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

CImageList CView::GetEventImageList() {
	return s_Images;
}

int CView::GetImageFromEventName(PCWSTR name) {
	if (auto it = s_IconsMap.find(name); it != s_IconsMap.end())
		return it->second;

	auto pos = ::wcschr(name, L'/');
	if (pos) {
		if (auto it = s_IconsMap.find(std::wstring(name, pos)); it != s_IconsMap.end())
			return it->second;
	}
	return 0;
}

void CView::OnFinalMessage(HWND /*hWnd*/) {
	GetFrame()->ViewDestroyed(this);
	delete this;
}

LRESULT CView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(*this, rcDefault, nullptr,
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_SINGLESEL | LVS_SHAREIMAGELISTS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER);

	auto processes = KernelEventCategory::GetCategory(L"Process");
	ATLASSERT(processes);

	// init events

	EventConfigCategory cat;
	cat.Name = processes->Name;
	m_EventsConfig.AddCategory(cat);

	// init filters

	FilterDescription desc;
	desc.Name = L"Process Id";
	desc.Action = FilterAction::Exclude;
	desc.Parameters = std::to_wstring(::GetCurrentProcessId());
	m_FilterConfig.AddFilter(desc);

	if (s_Images == nullptr) {
		s_Images.Create(16, 16, ILC_COLOR32, 8, 8);
		struct {
			int icon;
			PCWSTR type;
		} icons[] = {
			{ IDI_GENERIC, nullptr },
			{ IDI_HEAP2, L"PageFault/VirtualAlloc" },
			{ IDI_HEAP2, L"Virtual Memory" },
			{ IDI_GEAR, L"Process" },
			{ IDI_PROCESS_NEW, L"Process/Start" },
			{ IDI_PROCESS_DELETE, L"Process/Terminate" },
			{ IDI_PROCESS_DELETE, L"Process/End" },
			{ IDI_THREAD, L"Thread" },
			{ IDI_THREAD_NEW, L"Thread/Start" },
			{ IDI_THREAD_DELETE, L"Thread/End" },
			{ IDI_DLL, L"Image" },
			{ IDI_DLL_LOAD, L"Image/Load" },
			{ IDI_DLL_UNLOAD, L"Image/UnLoad" },
			{ IDI_NETWORK, L"Network" },
			{ IDI_NETWORK, L"TcpIp" },
			{ IDI_NETWORK, L"UdpIp" },
			{ IDI_REGISTRY, L"Registry" },
			{ IDI_FILE, L"FileIo" },
			{ IDI_FILE, L"File" },
			{ IDI_HANDLE, L"Object" },
			{ IDI_HANDLE, L"Objects" },
			{ IDI_OBJECT, L"Handles" },
			{ IDI_DISK, L"DiskIo" },
			{ IDI_DISK, L"Disk I/O" },
			{ IDI_MEMORY, L"PageFault" },
			{ IDI_MEMORY, L"Page Fault" },
			{ IDI_HEAP, L"Pool" },
		};
		int index = 0;
		for (auto entry : icons) {
			s_Images.AddIcon(AtlLoadIconImage(entry.icon, 0, 16, 16));
			if (entry.type)
				s_IconsMap.insert({ entry.type, index });
			index++;
		}
	}

	m_List.SetImageList(s_Images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"#", 0, 80);
	cm->AddColumn(L"Time", LVCFMT_RIGHT, 180);
	cm->AddColumn(L"Event", LVCFMT_LEFT, 160);
	cm->AddColumn(L"PID", LVCFMT_RIGHT, 100, ColumnFlags::Numeric | ColumnFlags::Visible);
	cm->AddColumn(L"Process Name", LVCFMT_LEFT, 150);
	cm->AddColumn(L"TID", LVCFMT_RIGHT, 100, ColumnFlags::Numeric | ColumnFlags::Visible);
	cm->AddColumn(L"Opcode", LVCFMT_CENTER, 45, ColumnFlags::Numeric);
	cm->AddColumn(L"Provider", LVCFMT_CENTER, 180, ColumnFlags::Numeric | ColumnFlags::Visible);
	cm->AddColumn(L"Details", LVCFMT_LEFT, 700);

	cm->UpdateColumns();

	m_TempEvents.reserve(1024);
	m_Events.reserve(4096);
	SetTimer(1, 1000, nullptr);

	return 0;
}

LRESULT CView::OnDestroy(UINT, WPARAM, LPARAM, BOOL& handled) {
	KillTimer(1);
	handled = FALSE;
	return 0;
}

LRESULT CView::OnForwardMsg(UINT, WPARAM, LPARAM lp, BOOL& handled) {
	auto msg = (MSG*)lp;
	LRESULT result;
	handled = ProcessWindowMessage(msg->hwnd, msg->message, msg->wParam, msg->lParam, result, 1);
	return result;
}

LRESULT CView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (m_IsMonitoring && id == 1) {
		if(!m_TempEvents.empty()) {
			std::lock_guard lock(m_EventsLock);
			m_Events.insert(m_Events.end(), m_TempEvents.begin(), m_TempEvents.end());
			m_OrgEvents.insert(m_OrgEvents.end(), m_TempEvents.begin(), m_TempEvents.end());
			m_TempEvents.clear();
		}
		else if (!m_IsMonitoring) {
			m_IsDraining = false;
		}
		UpdateList(m_List, static_cast<int>(m_Events.size()));
		if (m_AutoScroll)
			m_List.EnsureVisible(m_List.GetItemCount() - 1, FALSE);
		if (m_IsActive)
			UpdateEventStatus();
	}
	return 0;
}

LRESULT CView::OnClear(WORD, WORD, HWND, BOOL&) {
	CWaitCursor wait;
	m_List.SetItemCount(0);
	std::lock_guard lock(m_EventsLock);
	m_Events.clear();
	m_TempEvents.clear();
	m_OrgEvents.clear();
	return 0;
}

LRESULT CView::OnCallStack(WORD, WORD, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	if (selected < 0)
		return 0;

	auto data = m_Events[selected].get();
	if (data->GetStackEventData() == nullptr) {
		AtlMessageBox(*this, L"Call stack not available for this event", IDS_TITLE, MB_ICONEXCLAMATION);
		return 0;
	}

	CCallStackDlg dlg(data);
	dlg.DoModal();

	return 0;
}

LRESULT CView::OnEventProperties(WORD, WORD, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	if (selected < 0)
		return 0;

	auto data = m_Events[selected].get();
	CEventPropertiesDlg dlg(data);
	dlg.DoModal();

	return 0;
}

LRESULT CView::OnClose(UINT, WPARAM, LPARAM, BOOL& handled) {
	if (m_IsMonitoring) {
		AtlMessageBox(nullptr, L"Cannot close tab while monitoring is active", IDS_TITLE, MB_ICONWARNING);
		handled = TRUE;
	}
	handled = FALSE;
	return 0;
}

LRESULT CView::OnConfigureEvents(WORD, WORD, HWND, BOOL&) {
	CEventsDlg dlg(m_EventsConfig);
	if (dlg.DoModal() == IDOK) {
		if (m_IsMonitoring) {
			// update trace manager
			auto& tm = GetFrame()->GetTraceManager();
			std::vector<KernelEventTypes> types;
			std::vector<std::wstring> categories;
			for (auto& cat : m_EventsConfig.GetCategories()) {
				auto c = KernelEventCategory::GetCategory(cat.Name.c_str());
				ATLASSERT(c);
				types.push_back(c->EnableFlag);
				categories.push_back(c->Name);
			}
			std::initializer_list<KernelEventTypes> events(types.data(), types.data() + types.size());
			tm.SetKernelEventTypes(events);
			tm.SetKernelEventStacks(std::initializer_list<std::wstring>(categories.data(), categories.data() + categories.size()));
			tm.UpdateEventConfig();
		}
	}
	return 0;
}

LRESULT CView::OnAutoScroll(WORD, WORD, HWND, BOOL&) {
	m_AutoScroll = !m_AutoScroll;
	GetFrame()->GetUpdateUI()->UISetCheck(ID_VIEW_AUTOSCROLL, m_AutoScroll);

	return 0;
}

LRESULT CView::OnConfigFilters(WORD, WORD, HWND, BOOL&) {
	CFiltersDlg dlg(m_FilterConfig);
	if (dlg.DoModal() == IDOK) {
		// update filters
		auto& tm = GetFrame()->GetTraceManager();
		auto paused = tm.IsPaused();
		if(!paused)
			tm.Pause(true);
		tm.RemoveAllFilters();
		for (int i = 0; i < m_FilterConfig.GetFilterCount(); i++) {
			auto desc = m_FilterConfig.GetFilter(i);
			ATLASSERT(desc);
			auto filter = FilterFactory::CreateFilter(desc->Name.c_str(), desc->Compare, desc->Parameters.c_str(), desc->Action);
			ATLASSERT(filter);
			if (filter)
				tm.AddFilter(filter);
		}
		if(!paused)
			tm.Pause(false);
	}

	return 0;
}

LRESULT CView::OnItemChanged(int, LPNMHDR, BOOL&) {
	UpdateUI();

	return 0;
}

void CView::UpdateUI() {
	auto ui = GetFrame()->GetUpdateUI();
	ui->UISetCheck(ID_VIEW_AUTOSCROLL, m_AutoScroll);
	auto selected = m_List.GetSelectedIndex();
	ui->UIEnable(ID_EVENT_PROPERTIES, selected >= 0);
	ui->UIEnable(ID_EVENT_CALLSTACK, selected >= 0 && m_Events[selected]->GetStackEventData() != nullptr);
}
