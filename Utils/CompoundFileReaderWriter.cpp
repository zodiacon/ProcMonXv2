#include "pch.h"
#include "CompoundFileReaderWriter.h"

using namespace std;
using namespace StructuredStorage;

CompoundFileReaderWriter::CompoundFileReaderWriter(StructuredFile& file) : m_File(file) {
}

void CompoundFileReaderWriter::Write(const std::wstring & value) {
	auto len = static_cast<uint32_t>(value.size());
	m_File.Write(&len, sizeof(len));
	m_File.Write(value.c_str(), len * sizeof(wchar_t));
}

void CompoundFileReaderWriter::Read(std::wstring & value) {
	uint32_t len;
	m_File.Read(&len, sizeof(len));
	auto buffer = make_unique<wchar_t[]>(len);
	m_File.Read(buffer.get(), len * 2);
	value.assign(buffer.get(), len);
}

void CompoundFileReaderWriter::Write(const std::string & value) {
	auto len = static_cast<uint32_t>(value.size());
	m_File.Write(&len, sizeof(len));
	m_File.Write(value.c_str(), len * sizeof(char));
}

void CompoundFileReaderWriter::Read(std::string & value) {
	uint32_t len;
	m_File.Read(&len, sizeof(len));
	auto buffer = make_unique<char[]>(len);
	m_File.Read(buffer.get(), len);
	value.assign(buffer.get(), len);
}
