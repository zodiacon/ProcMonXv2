#include "pch.h"
#include "SerializerFactory.h"
#include "CSVEventDataSerializer.h"
#include "BinaryEventDataSerializer.h"

std::unique_ptr<IEventDataSerializer> SerializerFactory::CreateFromExtension(const CString& ext) {
    auto str(ext);
    str.MakeLower();

    if (str == "pmx")
        return std::make_unique<BinaryEventDataSerializer>();
    if (str == "csv")
        return std::make_unique<CSVEventDataSerializer>();

    return nullptr;
}
