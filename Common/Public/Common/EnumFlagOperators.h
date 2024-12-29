#pragma once

#include <Common/Platform/ForceInline.h>
#include <Common/TypeTraits/UnderlyingType.h>

#define ENUM_FLAG_OPERATORS(EnumType) \
	[[nodiscard]] FORCE_INLINE constexpr EnumType operator|(const EnumType left, const EnumType right) \
	{ \
		return static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) | static_cast<UNDERLYING_TYPE(EnumType)>(right)); \
	} \
\
	[[nodiscard]] FORCE_INLINE constexpr EnumType operator&(const EnumType left, const EnumType right) \
	{ \
		return static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) & static_cast<UNDERLYING_TYPE(EnumType)>(right)); \
	} \
\
	[[nodiscard]] FORCE_INLINE constexpr EnumType operator+(const EnumType left, const EnumType right) \
	{ \
		return static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) + static_cast<UNDERLYING_TYPE(EnumType)>(right)); \
	} \
\
	[[nodiscard]] FORCE_INLINE constexpr EnumType operator-(const EnumType left, const EnumType right) \
	{ \
		return static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) - static_cast<UNDERLYING_TYPE(EnumType)>(right)); \
	} \
\
	[[nodiscard]] FORCE_INLINE constexpr EnumType operator~(const EnumType right) \
	{ \
		return static_cast<EnumType>(~static_cast<UNDERLYING_TYPE(EnumType)>(right)); \
	} \
\
	FORCE_INLINE constexpr void operator|=(EnumType& left, const EnumType right) \
	{ \
		left = static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) | static_cast<UNDERLYING_TYPE(EnumType)>(right)); \
	} \
\
	FORCE_INLINE constexpr void operator&=(EnumType& left, const EnumType right) \
	{ \
		left = static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) & static_cast<UNDERLYING_TYPE(EnumType)>(right)); \
	} \
\
	[[nodiscard]] FORCE_INLINE constexpr EnumType operator<<(EnumType left, const UNDERLYING_TYPE(EnumType) right) \
	{ \
		return static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) << right); \
	} \
\
	[[nodiscard]] FORCE_INLINE constexpr EnumType operator>>(EnumType left, const UNDERLYING_TYPE(EnumType) right) \
	{ \
		return static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) >> right); \
	} \
\
	[[nodiscard]] FORCE_INLINE constexpr EnumType operator*(EnumType left, const bool right) \
	{ \
		return static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) * right); \
	} \
\
	[[nodiscard]] FORCE_INLINE constexpr EnumType operator+(EnumType left, const UNDERLYING_TYPE(EnumType) right) \
	{ \
		return static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) + right); \
	} \
\
	[[nodiscard]] FORCE_INLINE constexpr EnumType operator-(EnumType left, const UNDERLYING_TYPE(EnumType) right) \
	{ \
		return static_cast<EnumType>(static_cast<UNDERLYING_TYPE(EnumType)>(left) - right); \
	} \
\
	FORCE_INLINE constexpr EnumType& operator++(EnumType& value) \
	{ \
		value = EnumType(static_cast<UNDERLYING_TYPE(EnumType)>(value) + 1); \
		return value; \
	} \
\
	FORCE_INLINE constexpr EnumType& operator++(EnumType& value, int) \
	{ \
		value = EnumType(static_cast<UNDERLYING_TYPE(EnumType)>(value) + 1); \
		return value; \
	}
