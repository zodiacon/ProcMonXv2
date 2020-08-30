#pragma once

#include "IEventDataSerializer.h"
#include "CompoundFile.h"

class BinaryEventDataSerializer : public IEventDataSerializer {
public:
	// Inherited via IEventDataSerializer
	virtual bool Save(const std::vector<std::shared_ptr<EventData>>& events, const EventDataSerializerOptions& options, PCWSTR path) override;
	virtual std::vector<std::shared_ptr<EventData>> Load(PCWSTR path) override;

private:
	void WriteMetadata(StructuredStorage::CompoundFile* file, const std::vector<std::shared_ptr<EventData>>& events);
	void WriteEventData(StructuredStorage::StructuredDirectory* dir, const EventData* data);
	void WriteSimpleData(StructuredStorage::StructuredDirectory* dir, PCWSTR streamName, const void* data, int size);
	void WriteSimpleData(StructuredStorage::StructuredDirectory* dir, PCWSTR streamName, const std::wstring& value);
	template<typename T>
	void WriteSimpleData(StructuredStorage::StructuredDirectory* dir, PCWSTR streamName, const T& data) {
		static_assert(std::is_trivially_constructible<T>::value);
		WriteSimpleData(dir, streamName, &data, sizeof(data));
	}
};

