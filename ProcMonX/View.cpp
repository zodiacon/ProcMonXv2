// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "View.h"
#include "TraceManager.h"
#include "SortHelper.h"

CView::CView(IMainFrame* frame) : CViewBase(frame) {
}

void CView::AddEvent(std::shared_ptr<EventData> data) {
	if (data->GetEventName().empty())
		return;

	std::wstring details;
	for (auto& prop : data->GetProperties()) {
		if (prop.Name.substr(0, 8) != L"Reserved") {
			auto value = data->FormatProperty(prop);
			if (!value.empty()) {
				if (value.size() > 255)
					value = value.substr(0, 253) + L"...";
				details += prop.Name + L": " + value + L"; ";
			}
		}
	}
	data->SetDetails(details);
	{
		std::lock_guard lock(m_EventsLock);
		m_TempEvents.push_back(data);
	}
}

void CView::StartMonitoring(bool start) {
	m_IsMonitoring = start;
}

CString CView::GetColumnText(HWND, int row, int col) const {
	auto item = m_Events[row].get();
	CString text;

	switch (col) {
		case 0: 
		{
			auto ts = item->GetTimeStamp();
			text.Format(L".%06d", (ts / 10) % 1000000);
			return CTime(*(FILETIME*)&ts).Format(L"%x %X") + text;
		}
		case 1:
			return item->GetEventName().c_str();

		case 2: 
		{
			auto pid = item->GetProcessId();
			if (pid != (DWORD)-1)
				text.Format(L"%u (0x%X)", pid, pid);
			break;
		}
		case 3: return item->GetProcessId() == (DWORD)-1 ? L"" : GetFrame()->GetTraceManager().GetProcessImageById(item->GetProcessId()).c_str();
		case 4: 
		{
			auto tid = item->GetThreadId();
			if (tid != (DWORD)-1)
				text.Format(L"%u (0x%X)", tid, tid);
			break;
		}
		case 5: text.Format(L"%u", item->GetCPU()); break;
	}

	return text;
}

PCWSTR CView::GetColumnTextPointer(HWND, int row, int col) const {
	switch (col) {
		case 6: return  m_Events[row]->GetDetails().c_str();
	}
	return nullptr;
}

bool CView::IsSortable(int col) const {
	return !m_IsMonitoring && col != 6;
}

void CView::DoSort(const SortInfo* si) {
	std::sort(m_Events.begin(), m_Events.end(), [&](auto& i1, auto& i2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::SortNumbers(i1->GetTimeStamp(), i2->GetTimeStamp(), si->SortAscending);
			case 1: return SortHelper::SortStrings(i1->GetEventName(), i2->GetEventName(), si->SortAscending);
			case 2: return SortHelper::SortNumbers(i1->GetProcessId(), i2->GetProcessId(), si->SortAscending);
			case 3: return SortHelper::SortStrings(i1->GetProcessName(), i2->GetProcessName(), si->SortAscending);
			case 4: return SortHelper::SortNumbers(i1->GetThreadId(), i2->GetThreadId(), si->SortAscending);
			case 5: return SortHelper::SortNumbers(i1->GetCPU(), i2->GetCPU(), si->SortAscending);
		}
		return false;
		});
}

BOOL CView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

void CView::OnFinalMessage(HWND /*hWnd*/) {
	GetFrame()->ViewDestroyed(this);
	delete this;
}

LRESULT CView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_List.Create(*this, rcDefault, nullptr, 
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Time", LVCFMT_LEFT, 150);
	cm->AddColumn(L"Event", LVCFMT_LEFT, 100);
	cm->AddColumn(L"PID", LVCFMT_RIGHT, 100, ColumnFlags::Numeric | ColumnFlags::Visible);
	cm->AddColumn(L"Process Name", LVCFMT_LEFT, 150);
	cm->AddColumn(L"TID", LVCFMT_RIGHT, 100, ColumnFlags::Numeric | ColumnFlags::Visible);
	cm->AddColumn(L"CPU", LVCFMT_CENTER, 45, ColumnFlags::Numeric);
	cm->AddColumn(L"Details", LVCFMT_LEFT, 600);

	cm->UpdateColumns();

	m_TempEvents.reserve(1024);
	m_Events.reserve(4096);
	SetTimer(1, 1000, nullptr);

	return 0;
}

LRESULT CView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1 && !m_TempEvents.empty()) {
		{
			std::lock_guard lock(m_EventsLock);
			m_Events.insert(m_Events.end(), m_TempEvents.begin(), m_TempEvents.end());
			m_TempEvents.clear();
		}
		UpdateList(m_List, static_cast<int>(m_Events.size()));
	}
	return 0;
}
