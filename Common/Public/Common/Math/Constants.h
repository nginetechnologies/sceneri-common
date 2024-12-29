#pragma once

namespace ngine::Math
{
	template<typename T>
	struct TConstants
	{
		inline static constexpr T PI = static_cast<T>(3.14159265358979323846264338327950288419716939937510);
		inline static constexpr T PI2 = PI * static_cast<T>(2);

		inline static constexpr T RadToDeg = static_cast<T>(180) / PI;
		inline static constexpr T DegToRad = PI / static_cast<T>(180);

		//! Euler's number
		inline static constexpr T e = static_cast<T>(2.71828182845904523536028747135266249775724709369995);
	};

	using Constantsf = TConstants<float>;
	using Constantsd = TConstants<double>;
}
