#pragma once

namespace ngine::Math
{
	template<typename T>
	struct Epsilon
	{
		Epsilon(const T value)
			: m_value(value)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS operator T() const
		{
			return m_value;
		}

		T m_value;
	};
}
