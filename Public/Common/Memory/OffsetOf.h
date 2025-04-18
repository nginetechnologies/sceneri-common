#pragma once

#include <Common/Memory/AddressOf.h>
#include <Common/Platform/OffsetOf.h>
#include <Common/Platform/NoDebug.h>
#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Memory
{
	template<typename T, typename U>
	[[nodiscard]] FORCE_INLINE NO_DEBUG ptrdiff GetOffsetOf(U T::*member) noexcept
	{
		PUSH_CLANG_WARNINGS
		DISABLE_CLANG_WARNING("-Wnull-pointer-subtraction")
		return reinterpret_cast<const char*>(&(static_cast<T*>(nullptr)->*member)) - static_cast<char*>(nullptr);
		POP_CLANG_WARNINGS
	}

	template<typename T, typename U>
	[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr T& GetOwnerFromMember(U& impl, U T::*member) noexcept
	{
		const ptrdiff offsetFromOwner = GetOffsetOf(member);

		U* pImpl = GetAddressOf(impl);
		return *reinterpret_cast<T*>(reinterpret_cast<ptrdiff>(pImpl) - offsetFromOwner);
	}

	template<typename T, typename U>
	[[nodiscard]] FORCE_INLINE NO_DEBUG constexpr const T& GetConstOwnerFromMember(const U& impl, const U T::*member) noexcept
	{
		const ptrdiff offsetFromOwner = GetOffsetOf(member);

		U const * pImpl = GetAddressOf(impl);
		return *reinterpret_cast<const T*>(reinterpret_cast<ptrdiff>(pImpl) - offsetFromOwner);
	}
}
