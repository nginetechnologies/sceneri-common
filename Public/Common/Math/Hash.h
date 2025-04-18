#pragma once

#include <utility>

#include <Common/Memory/Containers/StringView.h>

PUSH_MSVC_WARNINGS_TO_LEVEL(2)
PUSH_CLANG_WARNINGS
DISABLE_CLANG_WARNING("-Wdeprecated-builtins")
DISABLE_CLANG_WARNING("-Wdeprecated")

#include <Common/3rdparty/absl/hash/hash.h>

POP_CLANG_WARNINGS
POP_MSVC_WARNINGS

namespace ngine::Math
{
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr size CombineHash(size left, const size right) noexcept
	{
		left ^= right + 0x9e3779b9 + (left << 6) + (left >> 2);
		return left;
	}

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS size Hash(const T& value) noexcept
	{
		absl::Hash<T> hasher;
		return hasher(value);
	}

	template<typename T, typename SizeType, typename IndexType, typename StoredType, uint8 Flags>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr size Hash(const ArrayView<T, SizeType, IndexType, StoredType, Flags> value) noexcept
	{
		size hash = 0;
		for (const T& element : value)
		{
			hash = CombineHash(hash, Hash(element));
		}

		return hash;
	}

	template<typename CharType, typename SizeType>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr size Hash(const TStringView<CharType, SizeType> value) noexcept
	{
		typename TStringView<CharType, SizeType>::Hash hash;
		return hash(value);
	}

	template<typename T0, typename T1>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr size Hash(const T0& a, const T1& b) noexcept
	{
		return CombineHash(Hash(a), Hash(b));
	}

	template<typename T0, typename T1, typename... Ts>
	[[nodiscard]] FORCE_INLINE PURE_NOSTATICS constexpr size Hash(const T0& val1, const T1& val2, const Ts&... vs)
	{
		return Hash(Hash(val1, val2), vs...);
	}
}
