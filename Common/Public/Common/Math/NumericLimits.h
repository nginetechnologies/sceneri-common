#pragma once

#include "CoreNumericTypes.h"

#include <Common/Memory/UnicodeCharType.h>

namespace ngine::Math
{
	template<typename T>
	struct NumericLimits
	{
	};

	template<>
	struct NumericLimits<unsigned char>
	{
		inline static constexpr unsigned char NumBits = sizeof(unsigned char) * CharBitCount;
		inline static constexpr unsigned char MinPositive = 0;
		inline static constexpr unsigned char Min = 0;
		inline static constexpr unsigned char Max = (unsigned char)~(unsigned char)0u;
		inline static constexpr unsigned char Epsilon = 0;
		inline static constexpr bool IsUnsigned = true;
	};

	template<>
	struct NumericLimits<signed char>
	{
		inline static constexpr signed char NumBits = sizeof(signed char) * CharBitCount;
		inline static constexpr signed char MinPositive = 0;
		inline static constexpr signed char Max = (signed char)(NumericLimits<unsigned char>::Max >> 1);
		inline static constexpr signed char Min = (-Max) - 1;
		inline static constexpr signed char Epsilon = 0;
		inline static constexpr bool IsUnsigned = false;
	};

#if IS_CHAR_UNSIGNED
	template<>
	struct NumericLimits<char> : public NumericLimits<unsigned char>
	{
	};
#else
	template<>
	struct NumericLimits<char> : public NumericLimits<signed char>
	{
	};
#endif

	template<>
	struct NumericLimits<unsigned short>
	{
		inline static constexpr unsigned short NumBits = sizeof(unsigned short) * CharBitCount;
		inline static constexpr unsigned short MinPositive = 0;
		inline static constexpr unsigned short Min = 0;
		inline static constexpr unsigned short Max = (unsigned short)~(unsigned short)0u;
		inline static constexpr unsigned short Epsilon = 0;
		inline static constexpr bool IsUnsigned = true;
	};

	template<>
	struct NumericLimits<signed short>
	{
		inline static constexpr signed short NumBits = sizeof(signed short) * CharBitCount;
		inline static constexpr signed short MinPositive = 0;
		inline static constexpr signed short Max = (signed short)(NumericLimits<unsigned short>::Max >> 1);
		inline static constexpr signed short Min = (-Max) - 1;
		inline static constexpr signed short Epsilon = 0;
		inline static constexpr bool IsUnsigned = false;
	};

	template<>
	struct NumericLimits<unsigned int>
	{
		inline static constexpr unsigned int NumBits = sizeof(unsigned int) * CharBitCount;
		inline static constexpr unsigned int MinPositive = 0;
		inline static constexpr unsigned int Min = 0;
		inline static constexpr unsigned int Max = (unsigned int)~(unsigned int)0u;
		inline static constexpr unsigned int Epsilon = 0;
		inline static constexpr bool IsUnsigned = true;
	};

	template<>
	struct NumericLimits<signed int>
	{
		inline static constexpr signed int NumBits = sizeof(signed int) * CharBitCount;
		inline static constexpr signed int MinPositive = 0;
		inline static constexpr signed int Max = (signed int)(NumericLimits<unsigned int>::Max >> 1);
		inline static constexpr signed int Min = (-Max) - 1;
		inline static constexpr signed int Epsilon = 0;
		inline static constexpr bool IsUnsigned = false;
	};

	template<>
	struct NumericLimits<unsigned long>
	{
		inline static constexpr unsigned long NumBits = sizeof(unsigned long) * CharBitCount;
		inline static constexpr unsigned long MinPositive = 0ul;
		inline static constexpr unsigned long Min = 0;
		inline static constexpr unsigned long Max = (unsigned long)~(unsigned long)0ul;
		inline static constexpr unsigned long Epsilon = 0;
		inline static constexpr bool IsUnsigned = true;
	};

	template<>
	struct NumericLimits<signed long>
	{
		inline static constexpr signed long NumBits = sizeof(signed long) * CharBitCount;
		inline static constexpr signed long MinPositive = 0;
		inline static constexpr signed long Max = (signed long)(NumericLimits<unsigned long>::Max >> 1);
		inline static constexpr signed long Min = (-Max) - 1;
		inline static constexpr signed long Epsilon = 0;
		inline static constexpr bool IsUnsigned = false;
	};

	template<>
	struct NumericLimits<unsigned long long>
	{
		inline static constexpr unsigned long long NumBits = sizeof(unsigned long long) * CharBitCount;
		inline static constexpr unsigned long long MinPositive = 0u;
		inline static constexpr unsigned long long Min = 0;
		inline static constexpr unsigned long long Max = (unsigned long long)~(unsigned long long)0ull;
		inline static constexpr unsigned long long Epsilon = 0;
		inline static constexpr bool IsUnsigned = true;
	};

	template<>
	struct NumericLimits<signed long long>
	{
		inline static constexpr signed long long NumBits = sizeof(signed long long) * CharBitCount;
		inline static constexpr signed long long MinPositive = 0;
		inline static constexpr signed long long Max = (signed long long)(NumericLimits<unsigned long long>::Max >> 1);
		inline static constexpr signed long long Min = (-Max) - 1;
		inline static constexpr signed long long Epsilon = 0;
		inline static constexpr bool IsUnsigned = false;
	};

	template<>
	struct NumericLimits<uint128>
	{
		inline static constexpr uint128 NumBits = 128;
		inline static constexpr uint128 MinPositive = 0;
		inline static constexpr uint128 Min = 0;
		inline static constexpr uint128 Max = ~uint128(0);
		inline static constexpr uint128 Epsilon = 0;
		inline static constexpr bool IsUnsigned = true;
	};

	template<>
	struct NumericLimits<int128>
	{
		inline static constexpr int128 NumBits = 128;
		inline static constexpr int128 MinPositive = 0;
		inline static constexpr int128 Max = (int128)((~uint128(0)) >> uint128(1));
#if __SIZEOF_INT128__
		inline static constexpr int128 Min = (-Max) - 1;
#else
		inline static constexpr int128 Min = (-Max)--;
#endif
		inline static constexpr int128 Epsilon = 0;
		inline static constexpr bool IsUnsigned = false;
	};

	template<>
	struct NumericLimits<float>
	{
		inline static constexpr uint16 NumBits = 32;
#if COMPILER_CLANG || COMPILER_GCC
		inline static constexpr float MinPositive = __FLT_MIN__;
		inline static constexpr float Max = __FLT_MAX__;
#elif COMPILER_MSVC
		inline static constexpr float MinPositive = 1.175494351e-38F;
		inline static constexpr float Max = 3.402823466e+38F;
#endif
		inline static constexpr float Min = -Max;
		inline static constexpr float Epsilon = 0.001f;
		inline static constexpr bool IsUnsigned = false;
	};

	template<>
	struct NumericLimits<double>
	{
		inline static constexpr uint16 NumBits = 64;
#if COMPILER_CLANG || COMPILER_GCC
		inline static constexpr double MinPositive = __DBL_MIN__;
		inline static constexpr double Max = __DBL_MAX__;
#elif COMPILER_MSVC
		inline static constexpr double MinPositive = 2.2250738585072014e-308;
		inline static constexpr double Max = 1.7976931348623158e+308;
#endif
		inline static constexpr double Min = -Max;
		inline static constexpr float Epsilon = 0.00001f;
		inline static constexpr bool IsUnsigned = false;
	};

#if PLATFORM_WINDOWS
	template<>
	struct NumericLimits<wchar_t> : public NumericLimits<uint16>
	{
		static_assert(sizeof(wchar_t) == sizeof(uint16));
	};
#endif

#if IS_UNICODE_CHAR8_UNIQUE_TYPE
	template<>
	struct NumericLimits<UTF8CharType> : public NumericLimits<uint8>
	{
		static_assert(sizeof(UTF8CharType) == sizeof(uint8));
	};
#endif
	template<>
	struct NumericLimits<char16_t> : public NumericLimits<uint16>
	{
		static_assert(sizeof(char16_t) == sizeof(uint16));
	};
	template<>
	struct NumericLimits<char32_t> : public NumericLimits<uint32>
	{
		static_assert(sizeof(char32_t) == sizeof(uint32));
	};
}
