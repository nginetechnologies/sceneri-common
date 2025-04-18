#pragma once

#if USE_WASM_SIMD128
#include <wasm_simd128.h>
#endif

namespace ngine::Math::Vectorization
{
#if USE_SSE
	template<>
	struct TRIVIAL_ABI Packed<uint16, 4> : public PackedBase<uint16, __m64, 4>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<uint16, __m64, 4>;
		using BaseType::BaseType;
	};
	template<>
	struct TRIVIAL_ABI Packed<int16, 4> : public PackedBase<int16, __m64, 4>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<int16, __m64, 4>;
		using BaseType::BaseType;
	};
#elif USE_NEON
	template<>
	struct TRIVIAL_ABI Packed<uint16, 4> : public PackedBase<uint16, uint16x4_t, 4>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<uint16, uint16x4_t, 4>;
		using BaseType::BaseType;
	};
	template<>
	struct TRIVIAL_ABI Packed<int16, 4> : public PackedBase<int16, int16x4_t, 4>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<int16, int16x4_t, 4>;
		using BaseType::BaseType;
	};
#elif USE_WASM_SIMD128
	template<>
	struct TRIVIAL_ABI Packed<uint16, 4> : public PackedBase<uint16, __u16x4, 4>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<uint16, __u16x4, 4>;
		using BaseType::BaseType;
	};
	template<>
	struct TRIVIAL_ABI Packed<int16, 4> : public PackedBase<int16, __i16x4, 4>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<int16, __i16x4, 4>;
		using BaseType::BaseType;
	};
#endif

#if USE_SSE
	template<>
	struct TRIVIAL_ABI Packed<uint16, 8> : public PackedBase<uint16, __m128i, 8>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<uint16, __m128i, 8>;
		using BaseType::BaseType;
	};
	template<>
	struct TRIVIAL_ABI Packed<int16, 8> : public PackedBase<int16, __m128i, 8>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<int16, __m128i, 8>;
		using BaseType::BaseType;
	};
#elif USE_NEON
	template<>
	struct TRIVIAL_ABI Packed<uint16, 8> : public PackedBase<uint16, uint16x8_t, 8>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<uint16, uint16x8_t, 8>;
		using BaseType::BaseType;
	};
	template<>
	struct TRIVIAL_ABI Packed<int16, 8> : public PackedBase<int16, int16x8_t, 8>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<int16, int16x8_t, 8>;
		using BaseType::BaseType;
	};
#elif USE_WASM_SIMD128
	template<>
	struct TRIVIAL_ABI Packed<uint16, 8> : public PackedBase<uint16, __u16x8, 8>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<uint16, __u16x8, 8>;
		using BaseType::BaseType;
	};
	template<>
	struct TRIVIAL_ABI Packed<int16, 8> : public PackedBase<int16, __i16x8, 8>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<int16, __i16x8, 8>;
		using BaseType::BaseType;
	};
#endif
}
