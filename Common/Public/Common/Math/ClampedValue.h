#pragma once

#include <Common/Math/Clamp.h>
#include <Common/Math/Ratio.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI ClampedValue
	{
		constexpr ClampedValue(const ClampedValue&) = default;
		constexpr ClampedValue(const T value, const T minimum, const T maximum)
			: m_value(value)
			, m_minimumValue(minimum)
			, m_maximumValue(maximum)
		{
		}
		constexpr ClampedValue& operator=(const ClampedValue&) = default;
		constexpr ClampedValue& operator=(const T value)
		{
			m_value = Math::Clamp(value, m_minimumValue, m_maximumValue);
			return *this;
		}

		[[nodiscard]] FORCE_INLINE operator T() const
		{
			return m_value;
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS constexpr Math::Ratiof GetRatio() const
		{
			const T value = m_value - m_minimumValue;
			return Math::Ratiof((float)value / (float)(m_maximumValue - m_minimumValue));
		}
		void SetRatio(const Math::Ratiof ratio)
		{
			m_value = m_minimumValue + T(float(m_maximumValue - m_minimumValue) * ratio);
		}

		bool Serialize(const Serialization::Reader);
		bool Serialize(Serialization::Writer) const;
	protected:
		T m_value;
		T m_minimumValue;
		T m_maximumValue;
	};

	struct ClampedValuef : public ClampedValue<float>
	{
		inline static constexpr Guid TypeGuid = "{07F13CB8-E81C-42EA-82FB-23FB0F36322D}"_guid;

		using BaseType = ClampedValue<float>;
		using BaseType::BaseType;
		using BaseType::operator=;
	};
}
