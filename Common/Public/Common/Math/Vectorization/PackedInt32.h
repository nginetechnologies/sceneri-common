#pragma once

#include <Common/Math/Vectorization/Packed.h>

#if USE_WASM_SIMD128
#include <wasm_simd128.h>
#endif

namespace ngine::Math::Vectorization
{
	template<>
	struct TRIVIAL_ABI Packed<uint32, 2> : public
#if USE_WASM_SIMD128
																				 PackedBase<uint32, __u32x2, 2>
#elif USE_SSE
																				 PackedBase<uint32, __m64, 2>
#elif USE_NEON
																				 PackedBase<uint32, uint32x2_t, 2>
#endif
	{
		inline static constexpr bool IsVectorized = false;

#if USE_WASM_SIMD128
		using BaseType = PackedBase<uint32, __u32x2, 2>;
#elif USE_SSE
		using BaseType = PackedBase<uint32, __m64, 2>;
#elif USE_NEON
		using BaseType = PackedBase<uint32, uint32x2_t, 2>;
#endif
		using BaseType::BaseType;
	};

	template<>
	struct TRIVIAL_ABI Packed<int32, 2> : public
#if USE_WASM_SIMD128
																				PackedBase<int32, __i32x2, 2>
#elif USE_SSE
																				PackedBase<int32, __m64, 2>
#elif USE_NEON
																				PackedBase<int32, int32x2_t, 2>
#endif
	{
		inline static constexpr bool IsVectorized = false;

#if USE_WASM_SIMD128
		using BaseType = PackedBase<int32, __i32x2, 2>;
#elif USE_SSE
		using BaseType = PackedBase<int32, __m64, 2>;
#elif USE_NEON
		using BaseType = PackedBase<int32, int32x2_t, 2>;
#endif
		using BaseType::BaseType;
	};

	template<>
	struct TRIVIAL_ABI Packed<uint32, 4> : public
#if USE_WASM_SIMD128
																				 PackedBase<uint32, __u32x4, 4>
#elif USE_SSE
																				 PackedBase<uint32, __m128i, 4>
#elif USE_NEON
																				 PackedBase<uint32, uint32x4_t, 4>
#endif
	{
		inline static constexpr bool IsVectorized = false;

#if USE_WASM_SIMD128
		using BaseType = PackedBase<uint32, __u32x4, 4>;
#elif USE_SSE
		using BaseType = PackedBase<uint32, __m128i, 4>;
#elif USE_NEON
		using BaseType = PackedBase<uint32, uint32x4_t, 4>;
#endif
		using BaseType::BaseType;
	};

	template<>
	struct TRIVIAL_ABI Packed<int32, 4> : public
#if USE_WASM_SIMD128
																				PackedBase<int32, __i32x4, 4>
#elif USE_SSE
																				PackedBase<int32, __m128i, 4>
#elif USE_NEON
																				PackedBase<int32, int32x4_t, 4>
#endif
	{
		inline static constexpr bool IsVectorized = USE_WASM_SIMD128 || USE_SSE || USE_NEON;

#if USE_WASM_SIMD128
		using BaseType = PackedBase<int32, __i32x4, 4>;
#elif USE_SSE
		using BaseType = PackedBase<int32, __m128i, 4>;
#elif USE_NEON
		using BaseType = PackedBase<int32, int32x4_t, 4>;
#endif
		using BaseType::BaseType;

		FORCE_INLINE Packed(const int32 value) noexcept
			: BaseType(
#if USE_WASM_SIMD128
					wasm_i32x4_splat(value)
#elif USE_SSE
					_mm_set1_epi32(value)
#elif USE_NEON
					vdupq_n_s32(value)
#endif
				)
		{
		}
		FORCE_INLINE explicit Packed(const Packed<float, 4> value)
			: BaseType(
#if USE_WASM_SIMD128
					value.m_value
#elif USE_SSE
					_mm_castps_si128(value)
#elif USE_NEON
					vreinterpretq_s32_f32(value)
#endif
				)
		{
		}
		FORCE_INLINE Packed(Math::ZeroType) noexcept
			: BaseType(
#if USE_WASM_SIMD128
					wasm_i32x4_const_splat(0)
#elif USE_SSE
					0
#elif USE_NEON
					vdupq_n_s32(0)
#endif
				)
		{
		}
		FORCE_INLINE Packed(const int32 value3, const int32 value2, const int32 value1, const int32 value0)
			: BaseType(
#if USE_WASM_SIMD128
					wasm_i32x4_make(value3, value2, value1, value0)
#elif USE_SSE
					_mm_set_epi32(value0, value1, value2, value3)
#elif USE_NEON
					int32x4_t{value3, value2, value1, value0}
#endif
				)
		{
		}

		FORCE_INLINE int32 GetSingle() noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_extract_lane(m_value, 0);
#elif USE_SSE
			return _mm_cvtsi128_si32(m_value);
#elif USE_NEON
			return vgetq_lane_s32(m_value, 0);
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed operator-() const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_neg(m_value);
#elif USE_SSE
			return _mm_xor_si128(m_value, _mm_set1_epi32(-0));
#elif USE_NEON
			return vnegq_s32(m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator+(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_add(m_value, other.m_value);
#elif USE_SSE
			return _mm_add_epi32(m_value, other);
#elif USE_NEON
			return vaddq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed& operator+=(const Packed other) noexcept
		{
			*this = *this + other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator-(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_sub(m_value, other.m_value);
#elif USE_SSE
			return _mm_sub_epi32(m_value, other);
#elif USE_NEON
			return vsubq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed& operator-=(const Packed other) noexcept
		{
			*this = *this - other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator*(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_mul(m_value, other.m_value);
#elif USE_SSE4_1
			return _mm_mul_epi32(m_value, other);
#elif USE_NEON
			return vmulq_s32(m_value, other.m_value);
#else
			return {
				m_values[0] * other.m_values[0],
				m_values[1] * other.m_values[1],
				m_values[2] * other.m_values[2],
				m_values[3] * other.m_values[3]
			};
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed& operator*=(const Packed other) noexcept
		{
			*this = *this * other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE UnitType Dot4Scalar(const Packed other) const
		{
			return m_values[0] * other.m_values[0] + m_values[1] * other.m_values[1] + m_values[2] * other.m_values[2] +
			       m_values[3] * other.m_values[3];
		}
		[[nodiscard]] FORCE_INLINE UnitType Dot3Scalar(const Packed other) const
		{
			return m_values[0] * other.m_values[0] + m_values[1] * other.m_values[1] + m_values[2] * other.m_values[2];
		}

		[[nodiscard]] FORCE_INLINE UnitType GetLengthSquared3Scalar() const
		{
			return Dot3Scalar(*this);
		}
		[[nodiscard]] FORCE_INLINE UnitType GetLengthSquared4Scalar() const
		{
			return Dot4Scalar(*this);
		}

		[[nodiscard]] FORCE_INLINE int GetMask() const noexcept
		{
#if USE_WASM_SIMD128
			return (int32)wasm_i32x4_bitmask(m_value);
#elif USE_SSE
			return _mm_movemask_ps(_mm_castsi128_ps(m_value));
#elif USE_NEON
			uint32x4_t mask = vreinterpretq_u32_s32(m_value);
			return vgetq_lane_u32(mask, 0) | vgetq_lane_u32(mask, 1) << 1 | vgetq_lane_u32(mask, 2) << 2 | vgetq_lane_u32(mask, 3) << 3;
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed operator/(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			alignas(16) int32_t elements[4];
			alignas(16) int32_t other_elements[4];
			alignas(16) int32_t result[4];

			wasm_v128_store(elements, m_value);
			wasm_v128_store(other_elements, other.m_value);

			for (int i = 0; i < 4; ++i)
			{
				result[i] = elements[i] / other_elements[i];
			}

			return Packed(wasm_v128_load(result));
#elif USE_SVML
			return _mm_div_epi32(m_value, other);
#elif USE_SSE
			alignas(16) int elements[4];
			alignas(16) int other_elements[4];
			alignas(16) int result[4];

			// Store SIMD register values into arrays
			_mm_store_si128(reinterpret_cast<__m128i*>(elements), m_value);
			_mm_store_si128(reinterpret_cast<__m128i*>(other_elements), other.m_value);

			// Perform element-wise division
			for (int i = 0; i < 4; ++i)
			{
				result[i] = elements[i] / other_elements[i];
			}

			// Load results back into a SIMD register
			return Packed{_mm_load_si128(reinterpret_cast<const __m128i*>(result))};
#elif USE_NEON
			alignas(16) int32_t elements[4];
			alignas(16) int32_t other_elements[4];
			alignas(16) int32_t result[4];
			vst1q_s32(elements, m_value);
			vst1q_s32(other_elements, other.m_value);
			for (int i = 0; i < 4; ++i)
			{
				result[i] = elements[i] / other_elements[i];
			}
			return Packed(vld1q_s32(result));
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed& operator/=(const Packed other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE Packed operator==(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_eq(m_value, other.m_value);
#elif USE_SSE
			return _mm_cmpeq_epi32(m_value, other.m_value);
#elif USE_NEON
			return vceqq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator!=(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_xor(wasm_i32x4_eq(m_value, other.m_value), wasm_i32x4_splat(-1));
#elif USE_SSE
			return _mm_xor_si128(_mm_cmpeq_epi32(m_value, other.m_value), _mm_set1_epi32(-1));
#elif USE_NEON
			return veorq_s32(vceqq_s32(m_value, other.m_value), vdupq_n_s32(-1));
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed operator>(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_gt(m_value, other.m_value);
#elif USE_SSE
			return _mm_cmpgt_epi32(m_value, other);
#elif USE_NEON
			return vcgtq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator>=(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_ge(m_value, other.m_value);
#elif USE_SSE
			return _mm_xor_si128(_mm_cmplt_epi32(m_value, other.m_value), _mm_set1_epi32(-1));
#elif USE_NEON
			return vmvnq_s32(vcltq_s32(m_value, other.m_value));
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator<(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_lt(m_value, other.m_value);
#elif USE_SSE
			return _mm_cmplt_epi32(m_value, other);
#elif USE_NEON
			return vcltq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator<=(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_le(m_value, other.m_value);
#elif USE_SSE
			return _mm_xor_si128(_mm_cmpgt_epi32(m_value, other.m_value), _mm_set1_epi32(-1));
#elif USE_NEON
			return vmvnq_s32(vcgtq_s32(m_value, other.m_value));
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator&(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_and(m_value, other.m_value);
#elif USE_SSE
			return _mm_and_si128(m_value, other);
#elif USE_NEON
			return vandq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator|(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_or(m_value, other.m_value);
#elif USE_SSE
			return _mm_or_si128(m_value, other);
#elif USE_NEON
			return vorrq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator^(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_xor(m_value, other.m_value);
#elif USE_SSE
			return _mm_xor_si128(m_value, other);
#elif USE_NEON
			return veorq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator~() const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_not(m_value);
#elif USE_SSE
			return _mm_xor_si128(m_value, _mm_set1_epi32(-1));
#elif USE_NEON
			return veorq_s32(m_value, vdupq_n_s32(-1));
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator!() const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_eq(m_value, wasm_i32x4_splat(0));
#elif USE_SSE
			__m128i zero = _mm_setzero_si128();
			return _mm_cmpeq_epi32(m_value, zero);
#elif USE_NEON
			return vceqq_s32(m_value, vdupq_n_s32(0));
#endif
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Packed operator>>(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			alignas(16) int32_t elements[4];
			alignas(16) int32_t shift_elements[4];
			alignas(16) int32_t result[4];

			wasm_v128_store(elements, m_value);
			wasm_v128_store(shift_elements, other.m_value);

			for (int i = 0; i < 4; ++i)
			{
				result[i] = elements[i] >> shift_elements[i];
			}

			return Packed(wasm_v128_load(result));
#elif USE_AVX2
			return _mm_srlv_epi32(m_value, other);
#elif USE_SSE
			return Packed(m_values[0] >> other[0], m_values[1] >> other[1], m_values[2] >> other[2], m_values[3] >> other[3]);
#elif USE_NEON
			return vshlq_s32(m_value, vnegq_s32(other.m_value));
#endif
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Packed operator<<(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			alignas(16) int32_t elements[4];
			alignas(16) int32_t shift_elements[4];
			alignas(16) int32_t result[4];

			wasm_v128_store(elements, m_value);
			wasm_v128_store(shift_elements, other.m_value);

			for (int i = 0; i < 4; ++i)
			{
				result[i] = elements[i] << shift_elements[i];
			}

			return Packed(wasm_v128_load(result));
#elif USE_AVX2
			return _mm_sllv_epi32(m_value, other);
#elif USE_SSE
			return Packed(m_values[0] << other[0], m_values[1] << other[1], m_values[2] << other[2], m_values[3] << other[3]);
#elif USE_NEON
			return vshlq_s32(m_value, other.m_value);
#endif
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Packed operator>>(const uint32 scalar) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_shr(m_value, scalar);
#elif USE_SSE2
			return _mm_srli_epi32(m_value, scalar);
#elif USE_NEON
			return vshlq_s32(m_value, vdupq_n_s32(-int32(scalar)));
#endif
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Packed operator<<(const uint32 scalar) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_i32x4_shl(m_value, scalar);
#elif USE_SSE2
			return _mm_slli_epi32(m_value, scalar);
#elif USE_NEON
			return vshlq_s32(m_value, vdupq_n_s32(int32(scalar)));
#endif
		}
	};

#if USE_AVX
	template<>
	struct TRIVIAL_ABI Packed<uint32, 8> : public PackedBase<uint32, __m256i, 8>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<uint32, __m256i, 8>;
		using BaseType::BaseType;
	};
	template<>
	struct TRIVIAL_ABI Packed<int32, 8> : public PackedBase<int32, __m256i, 8>
	{
		inline static constexpr bool IsVectorized = USE_AVX;
		using BaseType = PackedBase<int32, __m256i, 8>;
		using BaseType::BaseType;

		FORCE_INLINE Packed(const int32 value) noexcept
			: BaseType(_mm256_set1_epi32(value))
		{
		}
		FORCE_INLINE explicit Packed(const __m256 value)
			: BaseType(_mm256_castps_si256(value))
		{
		}
		FORCE_INLINE Packed(Math::ZeroType) noexcept
			: BaseType(0)
		{
		}
		FORCE_INLINE
		Packed(
			const int32 value7,
			const int32 value6,
			const int32 value5,
			const int32 value4,
			const int32 value3,
			const int32 value2,
			const int32 value1,
			const int32 value0
		)
			: BaseType(_mm256_set_epi32(value0, value1, value2, value3, value4, value5, value6, value7))
		{
		}

		[[nodiscard]] FORCE_INLINE Packed operator-() const noexcept
		{
			return _mm256_xor_si256(m_value, _mm256_set1_epi32(-0));
		}
		[[nodiscard]] FORCE_INLINE Packed operator+(const Packed other) const noexcept
		{
			return _mm256_add_epi32(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator+=(const Packed other) noexcept
		{
			*this = *this + other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator-(const Packed other) const noexcept
		{
			return _mm256_sub_epi32(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator-=(const Packed other) noexcept
		{
			*this = *this - other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator*(const Packed other) const noexcept
		{
			return _mm256_mul_epi32(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator*=(const Packed other) noexcept
		{
			*this = *this * other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE int GetMask() const noexcept
		{
#if USE_AVX
			return _mm256_movemask_ps(_mm256_castsi256_ps(m_value));
#elif USE_AVX2
			return _mm256_movemask_epi8(m_value);
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed operator/(const Packed other) const noexcept
		{
#if USE_SVML
			return _mm256_div_epi32(m_value, other);
#else
			alignas(32) int elements[8];
			alignas(32) int other_elements[8];
			alignas(32) int result[8];

			// Store SIMD register values into arrays
			_mm256_store_si256(reinterpret_cast<__m256i*>(elements), m_value);
			_mm256_store_si256(reinterpret_cast<__m256i*>(other_elements), other.m_value);

			// Perform element-wise division
			for (int i = 0; i < 8; ++i)
			{
				result[i] = elements[i] / other_elements[i];
			}

			// Load results back into a SIMD register
			return Packed{_mm256_load_si256(reinterpret_cast<const __m256i*>(result))};
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed& operator/=(const Packed other) noexcept
		{
			*this = *this / other;
			return *this;
		}

#if USE_AVX2
		[[nodiscard]] FORCE_INLINE Packed operator==(const Packed other) const noexcept
		{
			return _mm256_cmpeq_epi32(m_value, other.m_value);
		}
		[[nodiscard]] FORCE_INLINE Packed operator!=(const Packed other) const noexcept
		{
			return ~operator==(other);
		}
#else
		[[nodiscard]] FORCE_INLINE Packed operator==(const Packed other) const noexcept
		{
			const __m128i upper = _mm256_extractf128_si256(m_value, 0);
			const __m128i lower = _mm256_extractf128_si256(m_value, 1);
			const __m128i otherUpper = _mm256_extractf128_si256(other.m_value, 0);
			const __m128i otherLower = _mm256_extractf128_si256(other.m_value, 1);

			const __m128i eqUpper = _mm_cmpeq_epi32(upper, otherUpper);
			const __m128i eqLower = _mm_cmpeq_epi32(lower, otherLower);

			return _mm256_set_m128i(eqUpper, eqLower);
		}
		[[nodiscard]] FORCE_INLINE Packed operator!=(const Packed other) const noexcept
		{
			return ~operator==(other);
		}
#endif

		[[nodiscard]] FORCE_INLINE Packed operator>(const Packed other) const noexcept
		{
			return _mm256_cmpgt_epi32(m_value, other);
		}
		/*[[nodiscard]] FORCE_INLINE Packed operator>=(const Packed other) const noexcept
		{
		  return _mm_cmpge_epi32(m_value, other);
		}*/
		/*[[nodiscard]] FORCE_INLINE Packed operator<(const Packed other) const noexcept
		{
		  return _mm256_cmplt_epi32(m_value, other);
		}*/
		/*[[nodiscard]] FORCE_INLINE Packed operator<=(const Packed other) const noexcept
		{
		  return _mm_cmple_epi32(m_value, other);
		}*/
		[[nodiscard]] FORCE_INLINE Packed operator&(const Packed other) const noexcept
		{
			return _mm256_and_si256(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator|(const Packed other) const noexcept
		{
			return _mm256_or_si256(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator^(const Packed other) const noexcept
		{
			return _mm256_xor_si256(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator~() const noexcept
		{
			return _mm256_xor_si256(m_value, _mm256_set1_epi32(-1));
		}
		[[nodiscard]] FORCE_INLINE Packed operator!() const noexcept
		{
			__m256i zero = _mm256_setzero_si256();
			return _mm256_cmpeq_epi32(m_value, zero);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Packed operator>>(const Packed other) const noexcept
		{
			return _mm256_srlv_epi32(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Packed operator<<(const Packed other) const noexcept
		{
			return _mm256_sllv_epi32(m_value, other);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Packed operator>>(const uint32 scalar) const noexcept
		{
			return _mm256_srli_epi32(m_value, scalar);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS Packed operator<<(const uint32 scalar) const noexcept
		{
			return _mm256_slli_epi32(m_value, scalar);
		}
	};
#endif
}
