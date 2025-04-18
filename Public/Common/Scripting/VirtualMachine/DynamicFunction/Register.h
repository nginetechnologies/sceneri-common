#pragma once

#include <Common/Math/Vectorization/NativeTypes.h>
#include <Common/Math/CoreNumericTypes.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Assert/Assert.h>

namespace ngine::Scripting::VM
{
#if USE_WASM_SIMD128
	using Register = __f32x4;
#elif USE_AVX512
	using Register = __m512;
#elif USE_AVX
	using Register = __m256;
#elif USE_SSE
	using Register = __m128;
#elif USE_NEON
	using Register = float32x4_t;
#else
	using Register = uintptr;
#endif

	inline static constexpr uint8 RegisterCount = 6;

	struct TRIVIAL_ABI ReturnValue
	{
		[[nodiscard]] Register& operator[](const uint8 index)
		{
			Expect(index < 4);
			return *(&x + index);
		}
		[[nodiscard]] const Register& operator[](const uint8 index) const
		{
			Expect(index < 4);
			return *(&x + index);
		}

		Register x;
		Register y;
		Register z;
		Register w;
	};
}
