#pragma once

#include "Packed.h"

#if USE_WASM_SIMD128
#include <wasm_simd128.h>
#endif

namespace ngine::Math::Vectorization
{
	template<>
	struct TRIVIAL_ABI Packed<uint8, 8> : public
#if USE_WASM_SIMD128
																				PackedBase<uint8, __u8x8, 8>
#elif USE_SSE
																				PackedBase<uint8, __m64, 8>
#elif USE_NEON
																				PackedBase<uint8, uint8x8_t, 8>
#endif
	{
		inline static constexpr bool IsVectorized = false;

		using BaseType =
#if USE_WASM_SIMD128
			PackedBase<uint8, __u8x8, 8>;
#elif USE_SSE
			PackedBase<uint8, __m64, 8>;
#elif USE_NEON
			PackedBase<uint8, uint8x8_t, 8>;
#endif

		using BaseType::BaseType;
	};

	template<>
	struct TRIVIAL_ABI Packed<int8, 8> : public
#if USE_WASM_SIMD128
																			 PackedBase<int8, __i8x8, 8>
#elif USE_SSE
																			 PackedBase<int8, __m64, 8>
#elif USE_NEON
																			 PackedBase<int8, int8x8_t, 8>
#endif
	{
		inline static constexpr bool IsVectorized = false;

		using BaseType =
#if USE_WASM_SIMD128
			PackedBase<int8, __i8x8, 8>;
#elif USE_SSE
			PackedBase<int8, __m64, 8>;
#elif USE_NEON
			PackedBase<int8, int8x8_t, 8>;
#endif

		using BaseType::BaseType;
	};

	template<>
	struct TRIVIAL_ABI Packed<uint8, 16> : public
#if USE_WASM_SIMD128
																				 PackedBase<uint8, v128_t, 16>
#elif USE_SSE
																				 PackedBase<uint8, __m128i, 16>
#elif USE_NEON
																				 PackedBase<uint8, uint8x16_t, 16>
#endif
	{
		inline static constexpr bool IsVectorized = true;

		using BaseType =
#if USE_WASM_SIMD128
			PackedBase<uint8, v128_t, 16>;
#elif USE_SSE
			PackedBase<uint8, __m128i, 16>;
#elif USE_NEON
			PackedBase<uint8, uint8x16_t, 16>;
#endif

		using BaseType::BaseType;

		FORCE_INLINE Packed(const uint8 value) noexcept
			: BaseType(
#if USE_WASM_SIMD128
					wasm_i8x16_splat(value)
#elif USE_SSE
					_mm_set1_epi8(char(value))
#elif USE_NEON
					vdupq_n_u8(value)
#endif
				)
		{
		}

		FORCE_INLINE Packed(LoadUnalignedType, const uint8* value) noexcept
			: BaseType(
#if USE_WASM_SIMD128
					wasm_v128_load(value)
#elif USE_SSE
					_mm_loadu_si128(reinterpret_cast<const __m128i*>(value))
#elif USE_NEON
					vld1q_u8(value)
#endif
				)
		{
		}

		FORCE_INLINE Packed(LoadAlignedType, const uint8* value) noexcept
			: BaseType(
#if USE_WASM_SIMD128
					wasm_v128_load(value)
#elif USE_SSE
					_mm_load_si128(reinterpret_cast<const __m128i*>(value))
#elif USE_NEON
					vld1q_u8(value)
#endif
				)
		{
		}

		FORCE_INLINE Packed(Math::ZeroType) noexcept
			: BaseType(
#if USE_WASM_SIMD128
					wasm_i8x16_const_splat(0)
#elif USE_SSE
					_mm_set1_epi8(0)
#elif USE_NEON
					vdupq_n_u8(0)
#endif
				)
		{
		}

		FORCE_INLINE Packed(
			uint8 value0,
			uint8 value1,
			uint8 value2,
			uint8 value3,
			uint8 value4,
			uint8 value5,
			uint8 value6,
			uint8 value7,
			uint8 value8,
			uint8 value9,
			uint8 value10,
			uint8 value11,
			uint8 value12,
			uint8 value13,
			uint8 value14,
			uint8 value15
		)
			: BaseType(

#if USE_WASM_SIMD128
					wasm_i8x16_make(
						value15,
						value14,
						value13,
						value12,
						value11,
						value10,
						value9,
						value8,
						value7,
						value6,
						value5,
						value4,
						value3,
						value2,
						value1,
						value0
					)
#elif USE_SSE
					_mm_set_epi8(
						char(value15),
						char(value14),
						char(value13),
						char(value12),
						char(value11),
						char(value10),
						char(value9),
						char(value8),
						char(value7),
						char(value6),
						char(value5),
						char(value4),
						char(value3),
						char(value2),
						char(value1),
						char(value0)
					)
#elif USE_NEON
					vcombine_u8(
						vcreate_u8(
							uint64(value0) | (uint64(value1) << 8) | (uint64(value2) << 16) | (uint64(value3) << 24) | (uint64(value4) << 32) |
							(uint64(value5) << 40) | (uint64(value6) << 48) | (uint64(value7) << 56)
						),
						vcreate_u8(
							uint64(value8) | (uint64(value9) << 8) | (uint64(value10) << 16) | (uint64(value11) << 24) | (uint64(value12) << 32) |
							(uint64(value13) << 40) | (uint64(value14) << 48) | (uint64(value15) << 56)
						)
					)
#endif
				)
		{
		}

		[[nodiscard]] FORCE_INLINE Packed operator==(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return Packed(wasm_i8x16_eq(BaseType::m_value, other.BaseType::m_value));
#elif USE_SSE
			return Packed(_mm_cmpeq_epi8(BaseType::m_value, other.BaseType::m_value));
#elif USE_NEON
			return Packed(vceqq_u8(BaseType::m_value, other.BaseType::m_value));
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed operator|(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_or(m_value, other.m_value);
#elif USE_SSE
			return _mm_or_si128(m_value, other.m_value);
#elif USE_NEON
			return vorrq_u8(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator^(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_xor(m_value, other.m_value);
#elif USE_SSE
			return _mm_xor_si128(m_value, other.m_value);
#elif USE_NEON
			return veorq_u8(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator&(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_and(m_value, other.m_value);
#elif USE_SSE
			return _mm_and_si128(m_value, other.m_value);
#elif USE_NEON
			return vandq_u8(m_value, other.m_value);
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed operator~() const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_not(m_value);
#elif USE_SSE
			return _mm_xor_si128(m_value, _mm_set1_epi8(uint8(0xFF)));
#elif USE_NEON
			return vmvnq_u8(m_value);
#endif
		}

		[[nodiscard]] FORCE_INLINE int GetMask() const noexcept
		{
#if USE_WASM_SIMD128
			return (int)wasm_i8x16_bitmask(m_value);
#elif USE_SSE
			return _mm_movemask_epi8(m_value);
#else
			union
			{
				VectorizedType packed;
				uint8 bytes[16];
			} data;
			static_assert(sizeof(data) == sizeof(VectorizedType));
			data.packed = m_value;
			int result = 0;
			for (int i = 0; i < 16; ++i)
			{
				result |= int(data.bytes[i] >> 7) << i;
			}
			return result;
#endif
		}

		[[nodiscard]] FORCE_INLINE bool AreAllSet() const
		{
			return GetMask() == 0b1111111111111111;
		}
		[[nodiscard]] FORCE_INLINE bool AreAnySet() const
		{
			return GetMask() != 0;
		}
		[[nodiscard]] FORCE_INLINE bool AreNoneSet() const
		{
			return GetMask() == 0;
		}
	};

	template<>
	struct TRIVIAL_ABI Packed<int8, 16> : public
#if USE_WASM_SIMD128
																				PackedBase<int8, v128_t, 16>
#elif USE_SSE
																				PackedBase<int8, __m128i, 16>
#elif USE_NEON
																				PackedBase<int8, int8x16_t, 16>
#endif
	{
		inline static constexpr bool IsVectorized = false;

		using BaseType =
#if USE_WASM_SIMD128
			PackedBase<int8, v128_t, 16>;
#elif USE_SSE
			PackedBase<int8, __m128i, 16>;
#elif USE_NEON
			PackedBase<int8, int8x16_t, 16>;
#endif

		using BaseType::BaseType;
	};
}
