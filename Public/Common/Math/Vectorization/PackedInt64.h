#pragma once

#if USE_WASM_SIMD128
#include <wasm_simd128.h>
#endif

namespace ngine::Math::Vectorization
{
#if USE_WASM_SIMD128
	template<>
	struct TRIVIAL_ABI Packed<uint64, 2> : public PackedBase<uint64, __u64x2, 2>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<uint64, __u64x2, 2>;
		using BaseType::BaseType;
	};

	template<>
	struct TRIVIAL_ABI Packed<int64, 2> : public PackedBase<int64, __i64x2, 2>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<int64, __i64x2, 2>;
		using BaseType::BaseType;
	};
#elif USE_SSE
	template<>
	struct TRIVIAL_ABI Packed<uint64, 2> : public PackedBase<uint64, __m128i, 2>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<uint64, __m128i, 2>;
		using BaseType::BaseType;
	};

	template<>
	struct TRIVIAL_ABI Packed<int64, 2> : public PackedBase<int64, __m128i, 2>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<int64, __m128i, 2>;
		using BaseType::BaseType;
	};
#elif USE_NEON
	template<>
	struct TRIVIAL_ABI Packed<uint64, 2> : public PackedBase<uint64, uint64x2_t, 2>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<uint64, uint64x2_t, 2>;
		using BaseType::BaseType;
	};

	template<>
	struct TRIVIAL_ABI Packed<int64, 2> : public PackedBase<int64, int64x2_t, 2>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<int64, int64x2_t, 2>;
		using BaseType::BaseType;
	};
#endif
}
