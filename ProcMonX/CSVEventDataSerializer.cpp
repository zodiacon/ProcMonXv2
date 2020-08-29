#include "pch.h"
#include "CSVEventDataSerializer.h"
#include <fstream>

bool CSVEventDataSerializer::Save(const std::vector<std::shared_ptr<EventData>>& events, const EventDataSerializerOptions& options, PCWSTR path) {
    std::ofstream out;
    out.open(path);
    if(out.fail())
        return false;

    for (auto& evt : events) {
    }
    return false;
}

std::vector<std::shared_ptr<EventData>> CSVEventDataSerializer::Load(PCWSTR path) {
    return std::vector<std::shared_ptr<EventData>>();
}
