#include "pch.h"
#include "CompoundFile.h"

using namespace std;
using namespace StructuredStorage;

unique_ptr<CompoundFile> CompoundFile::Create(const std::wstring & path, PSECURITY_DESCRIPTOR securityDescriptor) {
	CComPtr<IStorage> spStg;
	auto hr = ::StgCreateStorageEx(path.c_str(), STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		STGFMT_STORAGE, 0, nullptr, securityDescriptor, __uuidof(IStorage), reinterpret_cast<void**>(&spStg));
	if (FAILED(hr))
		return nullptr;

	return unique_ptr<CompoundFile>(new CompoundFile(spStg));
}

unique_ptr<CompoundFile> CompoundFile::Open(const std::wstring & path, CompoundFileMode mode) {
	CComPtr<IStorage> spStg;
	auto hr = ::StgOpenStorageEx(path.c_str(), (mode == CompoundFileMode::Read ? STGM_READ : STGM_READWRITE) | STGM_SHARE_EXCLUSIVE,
		STGFMT_STORAGE, 0, nullptr, nullptr, __uuidof(IStorage), reinterpret_cast<void**>(&spStg));
	if (FAILED(hr))
		return nullptr;

	return unique_ptr<CompoundFile>(new CompoundFile(spStg, mode));
}

unique_ptr<StructuredDirectory> StructuredDirectory::CreateStructuredDirectory(const std::wstring & name) {
	CheckNameLength(name);

	CComPtr<IStorage> spStg;
	auto hr = GetStorage()->CreateStorage(name.c_str(), STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &spStg);
	if (FAILED(hr))
		return nullptr;

	return unique_ptr<StructuredDirectory>(new StructuredDirectory(spStg));
}

unique_ptr<StructuredFile> StructuredDirectory::CreateStructuredFile(const std::wstring & name) {
	CheckNameLength(name);

	CComPtr<IStream> spStm;
	auto hr = GetStorage()->CreateStream(name.c_str(), STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &spStm);
	if (FAILED(hr))
		return nullptr;

	return unique_ptr<StructuredFile>(new StructuredFile(spStm));
}

unique_ptr<StructuredFile> StructuredDirectory::OpenStructuredFile(const std::wstring & name) {
	CheckNameLength(name);

	CComPtr<IStream> spStm;
	auto hr = GetStorage()->OpenStream(name.c_str(), nullptr, 
		(GetMode() == CompoundFileMode::Read ? STGM_READ : STGM_READWRITE) | STGM_SHARE_EXCLUSIVE, 0, &spStm);
	if (FAILED(hr))
		return nullptr;

	return unique_ptr<StructuredFile>(new StructuredFile(spStm));
}

unique_ptr<StructuredDirectory> StructuredDirectory::OpenStructuredDirectory(const std::wstring & name) {
	CheckNameLength(name);

	CComPtr<IStorage> spStg;
	auto hr = GetStorage()->OpenStorage(name.c_str(), nullptr, 
		(GetMode() == CompoundFileMode::Read ? STGM_READ : STGM_READWRITE) | STGM_SHARE_EXCLUSIVE, 0, 0, &spStg);
	if (FAILED(hr))
		return nullptr;

	return unique_ptr<StructuredDirectory>(new StructuredDirectory(spStg, GetMode()));
}

void StructuredDirectory::Close() {
	m_spStorage = nullptr;
}

void StructuredDirectory::CheckNameLength(const std::wstring & name) const {
	if (name.size() > 31)
		throw exception("structured directory and file names must be less than 32 characters");
}

void StructuredFile::Write(const void * buffer, uint32_t count) {
	auto hr = m_spStream->Write(buffer, count, nullptr);
	if (FAILED(hr))
		throw ComException(hr);
}

void StructuredFile::Read(void * buffer, uint32_t count) {
	auto hr = m_spStream->Read(buffer, count, nullptr);
	if (FAILED(hr))
		throw ComException(hr);
}

uint32_t StructuredFile::Seek(uint32_t offset, SeekMode mode) {
	LARGE_INTEGER li;
	li.QuadPart = offset;
	ULARGE_INTEGER newOffset;
	auto hr = m_spStream->Seek(li, static_cast<DWORD>(mode), &newOffset);
	if (FAILED(hr))
		throw ComException(hr);

	return newOffset.LowPart;
}

uint32_t StructuredFile::GetSize() const {
	STATSTG stat = { 0 };
	m_spStream->Stat(&stat, STATFLAG_NONAME);
	return stat.cbSize.LowPart;
}

void StructuredFile::Close() {
	m_spStream = nullptr;
}
