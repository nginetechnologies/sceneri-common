#pragma once

#include <Common/Platform/NoDebug.h>

namespace ngine::Memory
{
	template<class Type>
	[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr Type* GetAddressOf(Type& value) noexcept
	{
		return (__builtin_addressof(value));
	}

	template<class Type>
	const Type* GetAddressOf(const Type&&) = delete;
}
