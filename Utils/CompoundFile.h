#pragma once

#include <objbase.h>
#include <string>
#include <atlcomcli.h>
#include <memory>

namespace StructuredStorage {
	enum class CompoundFileMode {
		Read,
		ReadWrite
	};

	enum class SeekMode {
		Set,
		Current,
		End
	};

	struct ComException : std::exception {
		ComException(HRESULT hr) : HResult(hr) {
		}

		HRESULT HResult;
	};

	class StructuredFile {
		friend class StructuredDirectory;

	public:
		void Write(const void* buffer, uint32_t count);
		void Read(void* buffer, uint32_t count);

		uint32_t Seek(uint32_t offset, SeekMode mode = SeekMode::Set);

		uint32_t GetSize() const;

		void Close();

	private:
		StructuredFile(IStream* pStm) : m_spStream(pStm) {}

		CComPtr<IStream> m_spStream;
	};

	class StructuredDirectory {
	public:
		std::unique_ptr<StructuredDirectory> CreateStructuredDirectory(const std::wstring& name);
		std::unique_ptr<StructuredFile> CreateStructuredFile(const std::wstring& name);
		std::unique_ptr<StructuredDirectory> OpenStructuredDirectory(const std::wstring& name);
		std::unique_ptr<StructuredFile> OpenStructuredFile(const std::wstring& name);

		void Close();

		~StructuredDirectory() {
			Close();
		}

		CompoundFileMode GetMode() const {
			return m_FileMode;
		}

	protected:
		IStorage* GetStorage() const {
			return m_spStorage.p;
		}

		StructuredDirectory(IStorage* pStg, CompoundFileMode mode = CompoundFileMode::ReadWrite) : m_spStorage(pStg), m_FileMode(mode) {
		}

		void CheckNameLength(const std::wstring& name) const;

	private:
		CComPtr<IStorage> m_spStorage;
		CompoundFileMode m_FileMode;
	};

	class CompoundFile : public StructuredDirectory {
	public:
		// factory methods

		static std::unique_ptr<CompoundFile> Create(const std::wstring& path, PSECURITY_DESCRIPTOR securityDescriptor = nullptr);
		static std::unique_ptr<CompoundFile> Open(const std::wstring& path, CompoundFileMode mode = CompoundFileMode::Read);

		// ctors

		CompoundFile(const CompoundFile&) = delete;
		CompoundFile& operator=(const CompoundFile&) = delete;

	private:
		CompoundFile(IStorage* pStg, CompoundFileMode mode = CompoundFileMode::ReadWrite) : StructuredDirectory(pStg, mode) {}

	};
}