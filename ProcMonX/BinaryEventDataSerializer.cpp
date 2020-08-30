#include "pch.h"
#include "BinaryEventDataSerializer.h"
#include "CompoundFile.h"
#include "CompoundFileReaderWriter.h"

using namespace StructuredStorage;

bool BinaryEventDataSerializer::Save(const std::vector<std::shared_ptr<EventData>>& events, const EventDataSerializerOptions& options, PCWSTR path) {
	try {
		auto file = CompoundFile::Create(path);
		if (file == nullptr)
			return false;

		uint32_t i = 0;
		CString text;
		WriteMetadata(file.get(), events);
		for (uint32_t i = 0; i < (uint32_t)events.size(); i++) {
			auto& evt = events[i];
			text.Format(L"Event%u", i);
			ATLTRACE(L"Writing event %u\n", i);
			auto dir = file->CreateStructuredDirectory((PCWSTR)text);
			WriteEventData(dir.get(), evt.get());
		}
	}
	catch (const ComException& ex) {
		ATLTRACE(L"Failed in BinaryEventDataSerializer::Save (HR=0x%X)\n", ex.HResult);
		return false;
	}

	if (options.CompressOutput) {
		// compress output file

	}

	return true;
}

std::vector<std::shared_ptr<EventData>> BinaryEventDataSerializer::Load(PCWSTR path) {
	return std::vector<std::shared_ptr<EventData>>();
}

void BinaryEventDataSerializer::WriteMetadata(CompoundFile* file, const std::vector<std::shared_ptr<EventData>>& events) {
	auto dir = file->CreateStructuredDirectory(L"Contents");
	WriteSimpleData(dir.get(), L"EventCount", events.size());
}

void BinaryEventDataSerializer::WriteEventData(StructuredDirectory* dir, const EventData* data) {
	auto file = dir->CreateStructuredFile(L"Basic");
	CompoundFileReaderWriter writer(*file);
	writer.Write(data->GetEventDescriptor());
	writer.Write(data->GetEventName());
	writer.Write(data->GetTimeStamp());
	writer.Write(data->GetIndex());
	writer.Write(data->GetProcessId());
	writer.Write(data->GetThreadId());
	writer.Write(data->GetProviderId());

	//WriteSimpleData(dir, L"EventDescriptor", &data->GetEventDescriptor(), sizeof(EVENT_DESCRIPTOR));
	//WriteSimpleData(dir, L"EventName", data->GetEventName());
	//WriteSimpleData(dir, L"Index", data->GetIndex());
	//WriteSimpleData(dir, L"ProcessId", data->GetProcessId());
	//WriteSimpleData(dir, L"ThreadId", data->GetThreadId());
	//WriteSimpleData(dir, L"ProcessName", data->GetProcessName());
	//WriteSimpleData(dir, L"ProviderId", data->GetProviderId());
	//WriteSimpleData(dir, L"TimeStamp", data->GetTimeStamp());
	auto props = data->GetProperties();
	if (!props.empty()) {
		auto file = dir->CreateStructuredFile(L"Properties");
		CompoundFileReaderWriter writer(*file);

		for (auto& prop : props) {
			writer.Write(prop.Name);
			writer.Write(prop.GetLength());
			file->Write(prop.GetData(), prop.GetLength());

			//WriteSimpleData(propDir.get(), prop.Name.c_str(), prop.GetData(), prop.GetLength());
		}
	}
	auto stackData = data->GetStackEventData();
	if (stackData) {
		auto stackDir = dir->CreateStructuredDirectory(L"Stack");
		WriteEventData(stackDir.get(), stackData);
	}
}

void BinaryEventDataSerializer::WriteSimpleData(StructuredDirectory* dir, PCWSTR streamName, const void* data, int size) {
	auto file = dir->CreateStructuredFile(streamName);
	file->Write(data, size);
}

void BinaryEventDataSerializer::WriteSimpleData(StructuredStorage::StructuredDirectory* dir, PCWSTR streamName, const std::wstring& value) {
	auto file = dir->CreateStructuredFile(streamName);
	CompoundFileReaderWriter writer(*file);
	writer.Write(value);
}
