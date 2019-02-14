#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Handle.hpp"

namespace rayx {
	class HandleManager {
		constexpr size_t MAX_ENTRIES = 4096; // 2^12
	public:
		HandleManager() {
			Reset();
		}

		void Reset() {
			m_activeEntryCount = 0;
			m_firstFreeEntry = 0;

			for (int i = 0; i < MAX_ENTRIES - 1; ++i) {
				m_entries[i] = HandleEntry(i + 1);
			}
			m_entries[MAX_ENTRIES - 1] = HandleEntry();
			m_entries[MAX_ENTRIES - 1].m_endOfList = true;
		}

		Handle Add(void* ptr, u32 type) {
			GUARANTEE_OR_DIE(m_activeEntryCount < MAX_ENTRIES - 1, "");
			GUARANTEE_OR_DIE(0 <= type && type <= 31, "");
			GUARANTEE_OR_DIE(m_firstFreeEntry < MAX_ENTRIES, "");
			GUARANTEE_OR_DIE(m_entries[m_firstFreeEntry].m_isActive == false, "Entry is in use");
			GUARANTEE_OR_DIE(!m_entries[m_firstFreeEntry].m_endOfList, "");

			u32 newIndex = m_firstFreeEntry;
			m_firstFreeEntry = m_entries[newIndex].m_nextFreeIndex;
			m_entries[newIndex].m_nextFreeIndex = 0;
			m_entries[newIndex].m_counter++;
			if (m_entries[newIndex].m_counter == 0) {
				m_entries[newIndex].m_counter = 1;
			}
			m_entries[newIndex].m_isActive = true;
			m_entries[newIndex].m_entry = ptr;

			m_activeEntryCount++;
			u32 counter = m_entries[newIndex];
			return Handle(newIndex, counter, type);
		}

		void Update(Handle handle, void* ptr) {
			u32 index = handle.m_index;
			GUARANTEE_OR_DIE(m_entries[index].m_counter == handle.m_counter, "");
			GUARANTEE_OR_DIE(m_entries[index].m_isActive == true, "");

			m_entries[index].m_entry = ptr;
		}

		void Remove(const Handle& handle) {
			u32 index = handle.m_index;
			GUARANTEE_OR_DIE(m_entries[index].m_counter == handle.m_counter, "");
			GUARANTEE_OR_DIE(m_entries[index].m_isActive == true, "");

			m_entries[index].m_nextFreeIndex = m_firstFreeEntry;
			m_entries[index].m_isActive = 0;
			m_firstFreeEntry = index;
			m_activeEntryCount--;
		}

		void* Get(const Handle handle) const {
			void* ptr = nullptr;
			if (!Get(handle, ptr)) {
				return nullptr;
			}
			return ptr;
		}

		bool Get(const Handle& handle, void* out) const {
			u32 index = handle.m_index;
			if (m_entries[index].m_counter != handle.m_counter || m_entries[index].m_isActive == false) {
				return false;
			}
			out = m_entries[index].m_entry;
			return true;
		}

	private:
		struct HandleEntry {
			HandleEntry()
				: m_nextFreeIndex(0)
				, m_counter(1)
				, m_isActive(0)
				, m_endOfList(0)
				, m_entry(nullptr) {}

			explicit HandleEntry(u32 nextFreeIndex)
				: HandleEntry()
				, m_nextFreeIndex(nextFreeIndex) {}

			u32 m_nextFreeIndex : 12;
			u32 m_counter : 15;
			u32 m_isActive : 1;
			u32 m_endOfList : 1;
			void* m_entry;
		};


		HandleEntry m_entries[MAX_ENTRIES];
		int m_activeEntryCount;
		u32 m_firstFreeEntry;
	};
}