#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Memory
{
	struct Size
	{
		[[nodiscard]] FORCE_INLINE constexpr static Size FromBytes(const size value)
		{
			return Size{value};
		}
		[[nodiscard]] FORCE_INLINE constexpr static Size FromKiloBytes(const size value)
		{
			return FromBytes(value * 1000);
		}
		[[nodiscard]] FORCE_INLINE constexpr static Size FromMegaBytes(const size value)
		{
			return FromKiloBytes(value * 1000);
		}
		[[nodiscard]] FORCE_INLINE constexpr static Size FromGigaBytes(const size value)
		{
			return FromMegaBytes(value * 1000);
		}

		[[nodiscard]] FORCE_INLINE constexpr size ToBytes() const
		{
			return m_value;
		}
	protected:
		constexpr Size(const size value)
			: m_value(value)
		{
		}

		size m_value;
	};

	namespace Literals
	{
		constexpr Size operator""_bytes(const unsigned long long value) noexcept
		{
			return Size::FromBytes((size)value);
		}
		constexpr Size operator""_kilobytes(const unsigned long long value) noexcept
		{
			return Size::FromKiloBytes((size)value);
		}
		constexpr Size operator""_megabytes(const unsigned long long value) noexcept
		{
			return Size::FromMegaBytes((size)value);
		}
		constexpr Size operator""_gigabytes(const unsigned long long value) noexcept
		{
			return Size::FromGigaBytes((size)value);
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Memory::Literals;
}
