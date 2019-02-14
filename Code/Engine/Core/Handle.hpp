#pragma once
#include "Engine/Core/type.hpp"

namespace rayx {
	struct Handle {
	public:
		Handle() : m_index(0), m_counter(0), m_type(0) {}
		Handle(u32 index, u32 counter, u32 type)
			: m_index(0), m_counter(counter), m_type(type) {}

		inline u32 GetAsU32() const;

		u32 m_index : 12;
		u32 m_counter : 15; // version
		u32 m_type : 5;
	};

	u32 Handle::GetAsU32() const {
		return m_type << 27 | m_counter << 12 | m_index;
	}
}