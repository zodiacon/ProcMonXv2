#include "pch.h"
#include "CSVEventDataSerializer.h"
#include "FormatHelper.h"
#include <fstream>
#include <format>

//  Needed for processes like msedge which contain commas in their command line, messing up the csv formatting
void CSVEventDataSerializer::EscapeCsvString(std::wstring& str)
{
    if (str.empty()) {
    	return; 
    }

    std::wstring escapedStr = L"";
    for (wchar_t c : str) {
        if (c == '\"') {
            escapedStr += c;
        }
        escapedStr += c;
    }

    str = std::format(L"\"{}\"", escapedStr);
}

std::wstring CSVEventDataSerializer::SerializeEvent(std::shared_ptr<EventData>& event)
{
    uint32_t index = event->GetIndex();
    DWORD tPid = event->GetProcessId();
    DWORD tTid = event->GetThreadId();

    std::wstring ts = (std::wstring)FormatHelper::FormatTime(event->GetTimeStamp());
    std::wstring name = event->GetEventName();
    std::wstring pName = event->GetProcessName();
    std::wstring pid = (tPid == (DWORD) - 1) ? L"" : std::to_wstring(tPid);
    std::wstring tid = (tTid == (DWORD) - 1) ? L"" : std::to_wstring(tTid);
    std::wstring details = L"";

    std::vector<EventProperty> props = event->GetProperties();

    if (props.size() > 0) {
        bool needsEscaping = false;
        for (EventProperty& p : props) {
            std::wstring pValue = event->FormatProperty(p);
            if (!pValue.empty()) { 
                details += p.Name + L": " + pValue + L"; ";
                if (p.Name == L"CommandLine") {
                    needsEscaping = true;
                }
            }      
        }

	if (needsEscaping) {
	    EscapeCsvString(details);
	}
    }

    return std::format(L"{},{},{},{},{},{},{}", index, ts, name, pName, pid, tid, details);
}

bool CSVEventDataSerializer::Save(const std::vector<std::shared_ptr<EventData>>& events, const EventDataSerializerOptions& options, PCWSTR path) {
    std::wofstream out;
    out.open(path);
    if(out.fail())
        return false;

    out << L"Id,Time Stamp,Event Name,Process Name,PID,TID,Details" << std::endl;

    for (std::shared_ptr<EventData> evt : events) {
        out << SerializeEvent(evt) << std::endl;
    }

    out.close();
    return true;
}

std::vector<std::shared_ptr<EventData>> CSVEventDataSerializer::Load(PCWSTR path) {
    return std::vector<std::shared_ptr<EventData>>();
}
