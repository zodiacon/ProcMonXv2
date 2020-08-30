#pragma once

#include "IEventDataSerializer.h"

struct SerializerFactory {
	static std::unique_ptr<IEventDataSerializer> CreateFromExtension(const CString& ext);
};

