#pragma once

#include <vector>
#include <map>

#include "CompoundFile.h"

namespace StructuredStorage {
	class CompoundFileReaderWriter {
	public:
		CompoundFileReaderWriter(StructuredFile& file);

		template<typename T>
		void Write(const T& value) {
			static_assert(std::is_trivially_copyable<T>(), "T must be POD");

			m_File.Write(&value, sizeof(value));
		}

		template<typename T>
		void Read(T& value) {
			static_assert(std::is_trivially_copyable<T>() , "T must be POD");

			m_File.Read(&value, sizeof(value));
		}

		void Write(const std::wstring& value);
		void Read(std::wstring& value);
		void Write(const std::string& value);
		void Read(std::string& value);

		template<typename T>
		void Write(const std::vector<T>& vec) {
			auto count = static_cast<uint32_t>(vec.size());
			Write(count);
			if (std::is_pod<T>::value) {
				m_File.Write(vec.data(), count * sizeof(T));
			}
			else {
				for (const auto& item : vec)
					Write(item);
			}
		}

		template<typename T>
		void Read(std::vector<T>& vec) {
			uint32_t count;
			Read(count);
			if (count == 0) {
				vec.clear();
				return;
			}

			vec.resize(count);
			if (std::is_pod<T>::value) {
				m_File.Read(vec.data(), count * sizeof(T));
			}
			else {
				for (uint32_t i = 0; i < count; ++i) {
					T value;
					Read(value);
					vec[i] = value;
				}
			}
		}

		template<typename T1, typename T2>
		void Write(const std::pair<T1, T2>& pair) {
			Write(pair.first);
			Write(pair.second);
		}

		template<typename T1, typename T2>
		void Read(std::pair<T1, T2>& pair) {
			Read(pair.first);
			Read(pair.second);
		}

		template<typename K, typename V>
		void Write(const std::map<K, V>& map) {
			Write(static_cast<uint32_t>(map.size()));
			for (const auto& pair : map)
				Write(pair);
		}

		template<typename K, typename V>
		void Read(std::map<K, V>& map) {
			uint32_t count;
			Read(count);
			for (uint32_t i = 0; i < count; i++) {
				std::pair<K, V> pair;
				Read(pair);
				map.insert(pair);
			}
		}

	private:
		StructuredFile& m_File;
	};

}
