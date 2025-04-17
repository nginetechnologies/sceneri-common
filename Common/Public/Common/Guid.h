#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Assert/Assert.h>
#include <Common/Platform/CompilerWarnings.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Platform/Likely.h>
#include <Common/Platform/Pure.h>
#include <Common/Memory/Containers/StringView.h>
#include <Common/Memory/Containers/ForwardDeclarations/FlatString.h>
#include <Common/TypeTraits/IsSame.h>
#include <Common/TypeTraits/EnableIf.h>

namespace ngine
{
	struct alignas(alignof(uint128)) TRIVIAL_ABI Guid
	{
		static constexpr const size ShortLength = 36; // XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
		static constexpr const size LongLength = 38;  // {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}

		template<typename CharType>
		[[nodiscard]] FORCE_INLINE static constexpr int ParseHexDigit(const CharType c)
		{
			if (('0' <= c) & (c <= '9'))
				return c - '0';
			else if (('a' <= c) & (c <= 'f'))
				return 10 + c - 'a';
			else if (('A' <= c) & (c <= 'F'))
				return 10 + c - 'A';
			return -1;
		}

		template<class T, typename CharType>
		[[nodiscard]] FORCE_INLINE static constexpr T ParseHex(const CharType* ptr)
		{
			constexpr uint8 digits = sizeof(T) * 2;
			T result{};
			for (uint8 i = 0; i < digits; ++i)
			{
				PUSH_CLANG_WARNINGS
				DISABLE_CLANG_WARNING("-Wsign-conversion");

				result |= ParseHexDigit(ptr[i]) << (4u * (digits - i - 1u));

				POP_CLANG_WARNINGS
			}

			return result;
		}

		template<typename CharType>
		[[nodiscard]] static constexpr Guid Parse(const CharType* begin)
		{
			bool isValid = true;
			isValid &= *(begin + 8) == '-';
			isValid &= *(begin + 13) == '-';
			isValid &= *(begin + 18) == '-';
			isValid &= *(begin + 23) == '-';

			Guid result;
			const uint32 data0 = ParseHex<uint32>(begin);
			result.m_data = uint128(data0) << 96;

			begin += 8 + 1;
			const uint16 data1 = ParseHex<uint16>(begin);
			result.m_data |= uint128(data1) << 80;

			begin += 4 + 1;
			const uint16 data2 = ParseHex<uint16>(begin);
			result.m_data |= uint128(data2) << 64;

			begin += 4 + 1;
			const uint16 data3 = ParseHex<uint16>(begin);
			result.m_data |= uint128(data3) << 48;

			begin += 4 + 1;
			for (uint8 i = 0; i < 6; ++i)
			{
				const uint8 data = ParseHex<uint8>(begin + i * 2);
				result.m_data |= uint128(data) << (40 - i * 8);
			}

			isValid &= result.IsValid();
			if (UNLIKELY(!isValid))
			{
				return {};
			}

			return result;
		}

		//! Generates a identifier from a string, based on an input base guid
		//! This is not really a Guid in that it's not globally unique - but sufficient for script parsing and more
		[[nodiscard]] static constexpr Guid FromString(const ConstStringView name, const Guid baseGuid)
		{
			Guid guid{baseGuid};
			uint128 data{guid.GetData()};
			for (uint8 index = 0; index < name.GetSize(); ++index)
			{
				// Calculate the position in m_data for XOR-ing, wrapped within the range of 16 bytes (128 bits)
				const uint8 normalizedIndex = (index % 16) * 8;

				// Extract the target byte, XOR it with the character, and put it back in m_data
				uint8 currentByte = static_cast<uint8>((data >> normalizedIndex) & 0xFF);
				currentByte ^= static_cast<uint8>(name[index]);
				data &= ~(uint128(0xFF) << normalizedIndex);                    // Clear the target byte
				data |= (static_cast<uint128>(currentByte) << normalizedIndex); // Set the new XOR-ed byte
			}

			return Guid{data};
		}
	public:
		constexpr Guid()
		{
		}
		constexpr Guid(const uint128 data)
			: m_data{data}
		{
		}
		template<size N>
		explicit constexpr Guid(const char input[N])
			: Guid(Parse(input + static_cast<uint8>(input[0] == '{')))
		{
			static_assert(N == ShortLength || N == LongLength);
			[[maybe_unused]] const uint8 offset = static_cast<uint8>(input[0] == '{');
			Assert(input[8 + offset] == '-' && input[13 + offset] == '-' && input[18 + offset] == '-' && input[23 + offset] == '-');
		}

		explicit constexpr Guid(const ConstStringView input)
			: Guid(Parse(input.GetData() + static_cast<uint8>(input[0] == '{')))
		{
			Assert(input.GetSize() == ShortLength || input.GetSize() == LongLength);
			[[maybe_unused]] const uint8 offset = static_cast<uint8>(input[0] == '{');
			Assert(input[8 + offset] == '-' && input[13 + offset] == '-' && input[18 + offset] == '-' && input[23 + offset] == '-');
		}

		template<typename NativeCharType_ = NativeCharType, typename = EnableIf<!TypeTraits::IsSame<char, NativeCharType_>>>
		explicit constexpr Guid(const ConstNativeStringView input)
			: Guid(Parse(input.GetData() + static_cast<uint8>(input[0] == '{')))
		{
			Assert(input.GetSize() == ShortLength || input.GetSize() == LongLength);
			[[maybe_unused]] const uint8 offset = static_cast<uint8>(input[0] == '{');
			Assert(input[8 + offset] == '-' && input[13 + offset] == '-' && input[18 + offset] == '-' && input[23 + offset] == '-');
		}

		static Guid TryParse(const ConstStringView input)
		{
			if (input.GetSize() == ShortLength || input.GetSize() == LongLength)
			{
				return Parse(input.GetData() + static_cast<uint8>(input[0] == '{'));
			}
			return Guid();
		}
		template<typename NativeCharType_ = NativeCharType, typename = EnableIf<!TypeTraits::IsSame<char, NativeCharType_>>>
		static Guid TryParse(const ConstNativeStringView input)
		{
			if (input.GetSize() == ShortLength || input.GetSize() == LongLength)
			{
				return Parse(input.GetData() + static_cast<uint8>(input[0] == '{'));
			}
			return Guid();
		}
		static Guid Generate();

		struct Hash
		{
			size operator()(const Guid& guid) const;
		};

		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const Guid other) const
		{
			return m_data == other.m_data;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const Guid other) const
		{
			return m_data != other.m_data;
		}
		[[nodiscard]] FORCE_INLINE bool operator<(const Guid other) const
		{
			return m_data < other.m_data;
		}

		[[nodiscard]] FORCE_INLINE constexpr bool IsValid() const
		{
			return m_data != 0;
		}

		[[nodiscard]] FORCE_INLINE bool IsInvalid() const
		{
			return !IsValid();
		}

		FlatString<37> ToString() const;
		[[nodiscard]] constexpr uint128 GetData() const
		{
			return m_data;
		}
	protected:
		uint128 m_data{0};
	};

	namespace Literals
	{
		constexpr ngine::Guid operator""_guid(const char* szInput, ngine::size n)
		{
			return ngine::Guid(ngine::ConstStringView(szInput, static_cast<ngine::uint32>(n)));
		}
	}

	using namespace Literals;
}
