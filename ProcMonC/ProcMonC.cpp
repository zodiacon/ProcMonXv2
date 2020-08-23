// ProcMonC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <tdh.h>
#include <stdio.h>
#include "TraceManager.h"
#include <memory>
#include "EventData.h"

void OnEvent(std::shared_ptr<EventData> data) {
	printf("%-15ws PID: %5u (%ws) TID: %5u",
		data->GetEventName().c_str(), data->GetProcessId(), data->GetProcessName().c_str(), data->GetThreadId());

	//switch (data->GetHeader().EventDescriptor.Opcode) {
	//	case 1:		// process created
	//		printf(" Created: %u (%s)", data->GetProperty(L"ProcessId")->GetValue<DWORD>(), data->GetProperty(L"ImageFileName")->GetAnsiString());
	//		break;
	//}
	printf("\n");
}

TraceManager* g_pMgr;
HANDLE g_hEvent;

int main(int argc, const char* argv[]) {
	TraceManager tm;
	tm.AddKernelEventTypes({ KernelEventTypes::ImageLoad, KernelEventTypes::Process });

	g_pMgr = &tm;
	g_hEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);

	if (!tm.Start(OnEvent)) {
		printf("Failed to start session");
		return 1;
	}

	::SetConsoleCtrlHandler([](auto type) {
		if (type == CTRL_C_EVENT) {
			g_pMgr->Stop();
			::SetEvent(g_hEvent);
			return TRUE;
		}
		return FALSE;
		}, TRUE);

	::WaitForSingleObject(g_hEvent, INFINITE);
	::CloseHandle(g_hEvent);

	return 0;
}

