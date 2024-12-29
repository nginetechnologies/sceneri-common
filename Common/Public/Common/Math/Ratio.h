#pragma once

#include <Common/Math/ForwardDeclarations/Ratio.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Guid.h>

#include <Common/Math/MultiplicativeInverse.h>
#include <Common/Math/Epsilon.h>

#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TRatio
	{
		inline static constexpr Guid TypeGuid = "{D6B96F78-D951-400E-A712-CE35B4A5F3BC}"_guid;

		constexpr TRatio() = default;
		FORCE_INLINE constexpr TRatio(const T ratio) noexcept
			: m_ratio(ratio)
		{
		}

		template<typename OtherType>
		FORCE_INLINE constexpr TRatio(const TRatio<OtherType> ratio) noexcept
			: m_ratio(static_cast<T>((OtherType)ratio))
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr operator T() const noexcept
		{
			return m_ratio;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr TRatio GetInverted() const noexcept
		{
			return TRatio(T(1) - m_ratio);
		}

		template<typename OtherType>
		FORCE_INLINE constexpr TRatio& operator+=(const TRatio<OtherType> other) noexcept
		{
			m_ratio += static_cast<T>((OtherType)other);
			return *this;
		}

		template<typename OtherType>
		FORCE_INLINE constexpr PURE_STATICS TRatio operator+(const TRatio<OtherType> other) const noexcept
		{
			return TRatio(m_ratio + static_cast<T>((OtherType)other));
		}

		template<typename OtherType>
		FORCE_INLINE constexpr TRatio& operator-=(const TRatio<OtherType> other) noexcept
		{
			m_ratio -= static_cast<T>((OtherType)other);
			return *this;
		}

		template<typename OtherType>
		FORCE_INLINE constexpr PURE_STATICS TRatio operator-(const TRatio<OtherType> other) const noexcept
		{
			return TRatio(m_ratio - static_cast<T>((OtherType)other));
		}

		FORCE_INLINE constexpr PURE_STATICS T operator*(const T scalar) const noexcept
		{
			return m_ratio * scalar;
		}

		template<typename OtherType>
		FORCE_INLINE constexpr PURE_STATICS TRatio operator*(const TRatio<OtherType> other) const noexcept
		{
			return TRatio(m_ratio * (T)other);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS TRatio operator-() const
		{
			return TRatio(-m_ratio);
		}

		bool Serialize(const Serialization::Reader);
		bool Serialize(Serialization::Writer) const;
	protected:
		T m_ratio;
	};

	template<typename T>
	FORCE_INLINE constexpr PURE_STATICS T operator*(const T scalar, const TRatio<T> ratio) noexcept
	{
		return (T)ratio * scalar;
	}

	namespace Literals
	{
		constexpr Ratiod operator""_percent(unsigned long long value) noexcept
		{
			return Ratiod(static_cast<double>(value) / 100.0);
		}

		constexpr Ratiod operator""_percent(long double value) noexcept
		{
			return Ratiod(static_cast<double>(value) / 100.0);
		}

		constexpr Ratiod operator""_ratio(unsigned long long value) noexcept
		{
			return Ratiod(static_cast<double>(value));
		}

		constexpr Ratiod operator""_ratio(long double value) noexcept
		{
			return Ratiod(static_cast<double>(value));
		}
	}

	using namespace Literals;

	template<typename T>
	[[nodiscard]] FORCE_INLINE PURE_STATICS Math::TRatio<T> MultiplicativeInverse(const Math::TRatio<T> value) noexcept
	{
		return Math::TRatio<T>(Math::MultiplicativeInverse((T)value));
	}

	template<typename T>
	struct NumericLimits<TRatio<T>>
	{
		inline static constexpr TRatio<T> NumBits = TRatio<T>(NumericLimits<T>::NumBits);
		inline static constexpr TRatio<T> Min = TRatio<T>(NumericLimits<T>::Min);
		inline static constexpr TRatio<T> Max = TRatio<T>(NumericLimits<T>::Max);
		inline static constexpr TRatio<T> Epsilon = TRatio<T>(NumericLimits<T>::Epsilon);
		inline static constexpr bool IsUnsigned = NumericLimits<T>::IsUnsigned;
	};
}

namespace ngine
{
	using namespace Math::Literals;
}
