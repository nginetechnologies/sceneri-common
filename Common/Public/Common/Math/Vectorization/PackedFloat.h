#pragma once

#include <Common/Memory/Containers/FixedArrayView.h>
#include <Common/Math/Sqrt.h>

#if USE_WASM_SIMD128
#include <wasm_simd128.h>
#endif

namespace ngine::Math::Vectorization
{
	template<>
	struct TRIVIAL_ABI Packed<float, 2> : public
#if USE_WASM_SIMD128
																				PackedBase<float, __f32x2, 2>
#elif USE_SSE
																				PackedBase<float, __m64, 2>
#elif USE_NEON
																				PackedBase<float, float32x2_t, 2>
#endif
	{
		inline static constexpr bool IsVectorized = false;

#if USE_WASM_SIMD128
		using BaseType = PackedBase<float, __f32x2, 2>;
#elif USE_SSE
		using BaseType = PackedBase<float, __m64, 2>;
#elif USE_NEON
		using BaseType = PackedBase<float, float32x2_t, 2>;
#endif
		using BaseType::BaseType;
	};

	template<>
	struct TRIVIAL_ABI Packed<float, 4> : public
#if USE_WASM_SIMD128
																				PackedBase<float, __f32x4, 4>
#elif USE_SSE
																				PackedBase<float, __m128, 4>
#elif USE_NEON
																				PackedBase<float, float32x4_t, 4>
#endif
	{
		inline static constexpr bool IsVectorized = USE_SSE || USE_NEON || USE_WASM_SIMD128;

#if USE_WASM_SIMD128
		using BaseType = PackedBase<float, __f32x4, 4>;
#elif USE_SSE
		using BaseType = PackedBase<float, __m128, 4>;
#elif USE_NEON
		using BaseType = PackedBase<float, float32x4_t, 4>;
#endif

		using BaseType::BaseType;
		FORCE_INLINE Packed(const SetSingleType, const float value) noexcept
			: BaseType(
#if USE_WASM_SIMD128
					wasm_f32x4_make(0.f, 0.f, 0.f, value)
#elif USE_SSE
					_mm_set_ps1(value)
#elif USE_NEON
					float32x4_t{0.f, 0.f, 0.f, value}
#endif
				)
		{
		}
		FORCE_INLINE Packed(const float value) noexcept
			: BaseType(
#if USE_WASM_SIMD128
					wasm_f32x4_splat(value)
#elif USE_SSE
					_mm_set_ss(value)
#elif USE_NEON
					vdupq_n_f32(value)
#endif
				)
		{
		}
		FORCE_INLINE Packed(Math::ZeroType) noexcept
			: BaseType(
#if USE_WASM_SIMD128
					wasm_f32x4_const_splat(0.f)
#elif USE_SSE
					0.f
#elif USE_NEON
					vdupq_n_f32(0.f)
#endif
				)
		{
		}
		FORCE_INLINE Packed(const float value3, const float value2, const float value1, const float value0)
			: BaseType(
#if USE_WASM_SIMD128
					wasm_f32x4_make(value3, value2, value1, value0)
#elif USE_SSE
					_mm_set_ps(value0, value1, value2, value3)
#elif USE_NEON
					float32x4_t{value3, value2, value1, value0}
#endif
				)
		{
		}

		FORCE_INLINE void StoreSingle(float& out) noexcept
		{
#if USE_WASM_SIMD128
			out = wasm_f32x4_extract_lane(m_value, 0);
#elif USE_SSE
			_mm_store_ss(&out, m_value);
#elif USE_NEON
			out = vgetq_lane_f32(m_value, 0);
#endif
		}

		FORCE_INLINE float GetSingle() noexcept
		{
#if USE_WASM_SIMD128
			return wasm_f32x4_extract_lane(m_value, 0);
#elif USE_SSE
			return _mm_cvtss_f32(m_value);
#elif USE_NEON
			return vgetq_lane_f32(m_value, 0);
#endif
		}

		FORCE_INLINE void StoreAll(const FixedArrayView<float, 4> out) noexcept
		{
#if USE_WASM_SIMD128
			wasm_v128_store(out.GetData(), m_value);
#elif USE_SSE
			_mm_store_ps(out.GetData(), m_value);
#elif USE_NEON
			vst1q_f32(out.GetData(), m_value);
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed operator-() const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_f32x4_neg(m_value);
#elif USE_SSE
			return _mm_xor_ps(m_value, _mm_set1_ps(-0.0));
#elif USE_NEON
			return vmulq_n_f32(m_value, -1.0f);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator+(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_f32x4_add(m_value, other.m_value);
#elif USE_SSE
			return _mm_add_ps(m_value, other);
#elif USE_NEON
			return vaddq_f32(m_value, other.m_value);
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
			return wasm_f32x4_sub(m_value, other.m_value);
#elif USE_SSE
			return _mm_sub_ps(m_value, other);
#elif USE_NEON
			return vsubq_f32(m_value, other.m_value);
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
			return wasm_f32x4_mul(m_value, other.m_value);
#elif USE_SSE
			return _mm_mul_ps(m_value, other);
#elif USE_NEON
			return vmulq_f32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed& operator*=(const Packed other) noexcept
		{
			*this = *this * other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator/(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_f32x4_div(m_value, other.m_value);
#elif USE_SSE
			return _mm_div_ps(m_value, other);
#elif USE_NEON
			return vdivq_f32(m_value, other.m_value);
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
			return wasm_f32x4_eq(m_value, other.m_value);
#elif USE_SSE
			return _mm_cmpeq_ps(m_value, other.m_value);
#elif USE_NEON
			return vceqq_f32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator!=(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_f32x4_ne(m_value, other.m_value);
#elif USE_SSE
			return _mm_cmpneq_ps(m_value, other.m_value);
#elif USE_NEON
			return vmvnq_u32(vceqq_f32(m_value, other.m_value));
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed operator>(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_f32x4_gt(m_value, other.m_value);
#elif USE_SSE
			return _mm_cmpgt_ps(m_value, other);
#elif USE_NEON
			return vcgtq_f32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator>=(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_f32x4_ge(m_value, other.m_value);
#elif USE_SSE
			return _mm_cmpge_ps(m_value, other);
#elif USE_NEON
			return vcgeq_f32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator<(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_f32x4_lt(m_value, other.m_value);
#elif USE_SSE
			return _mm_cmplt_ps(m_value, other);
#elif USE_NEON
			return vcltq_f32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator<=(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_f32x4_le(m_value, other.m_value);
#elif USE_SSE
			return _mm_cmple_ps(m_value, other);
#elif USE_NEON
			return vcleq_f32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator&(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_and(m_value, other.m_value);
#elif USE_SSE
			return _mm_and_ps(m_value, other);
#elif USE_NEON
			return vandq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator|(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_or(m_value, other.m_value);
#elif USE_SSE
			return _mm_or_ps(m_value, other);
#elif USE_NEON
			return vorrq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator^(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_xor(m_value, other.m_value);
#elif USE_SSE
			return _mm_xor_ps(m_value, other);
#elif USE_NEON
			return veorq_s32(m_value, other.m_value);
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator~() const noexcept
		{
#if USE_WASM_SIMD128
			return wasm_v128_not(m_value);
#elif USE_SSE
			return _mm_xor_ps(m_value, _mm_castsi128_ps(_mm_set1_epi32(-1)));
#elif USE_NEON
			return veorq_s32(m_value, vdupq_n_f32(-1));
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed operator!() const noexcept
		{
#if USE_WASM_SIMD128
			const __f32x4 zero = wasm_f32x4_splat(0.0f);
			return wasm_f32x4_eq(m_value, zero);
#elif USE_SSE
			__m128 zero = _mm_setzero_ps();
			__m128 cmp = _mm_cmpeq_ps(m_value, zero);
			return _mm_xor_ps(cmp, _mm_set1_ps(1.0f));
#elif USE_NEON
			const float32x4_t zero = vdupq_n_f32(0.0f);
			return vreinterpretq_f32_u32(vceqq_f32(m_value, zero));
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed GetHorizontalSum() const noexcept
		{
#if USE_WASM_SIMD128
			__f32x4 temp = wasm_f32x4_add(m_value, wasm_i32x4_shuffle(m_value, m_value, 2, 3, 0, 1));
			temp = wasm_f32x4_add(temp, wasm_i32x4_shuffle(temp, temp, 1, 0, 3, 2));
			return wasm_f32x4_extract_lane(temp, 0);
#elif USE_SSE
			const Packed zwxy = _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(1, 0, 3, 2));
			const Packed h = _mm_add_ps(*this, zwxy);
			const Packed yxyx = _mm_shuffle_ps(h, h, _MM_SHUFFLE(0, 1, 0, 1));
			return _mm_add_ps(h, yxyx);
#elif USE_NEON
			float32x2_t pairSum = vadd_f32(vget_low_f32(m_value), vget_high_f32(m_value));
			float32x2_t finalSum = vpadd_f32(pairSum, pairSum);
			return vdupq_lane_f32(finalSum, 0);
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed Dot4(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			__f32x4 product = wasm_f32x4_mul(m_value, other.m_value);
			float temp[4];
			wasm_v128_store(temp, product);
			return Packed(temp[0] + temp[1] + temp[2] + temp[3]);
#elif USE_SSE4_1
			return _mm_dp_ps(m_value, other, 0xff);
#elif USE_NEON
			float32x4_t mul = vmulq_f32(m_value, other.m_value);
			return vdupq_n_f32(vaddvq_f32(mul));
#else
			return Packed{
				m_values[0] * other.m_values[0] + m_values[1] * other.m_values[1] + m_values[2] * other.m_values[2] +
				m_values[3] * other.m_values[3]
			};
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed Dot3(const Packed other) const noexcept
		{
#if USE_WASM_SIMD128
			__f32x4 product = wasm_f32x4_mul(m_value, other.m_value);
			float temp[4];
			wasm_v128_store(temp, product);
			return Packed(temp[0] + temp[1] + temp[2]);
#elif USE_SSE4_1
			return _mm_dp_ps(m_value, other, 0b01110111);
#elif USE_NEON
			float32x4_t mul = vmulq_f32(m_value, other.m_value);
			mul = vsetq_lane_f32(0, mul, 3);
			return vaddvq_f32(mul);
#else
			return Packed{m_values[0] * other.m_values[0] + m_values[1] * other.m_values[1] + m_values[2] * other.m_values[2]};
#endif
		}
		[[nodiscard]] FORCE_INLINE float Dot4Scalar(const Packed other) const
		{
#if USE_WASM_SIMD128
			__f32x4 product = wasm_f32x4_mul(m_value, other.m_value);
			float temp[4];
			wasm_v128_store(temp, product);
			return temp[0] + temp[1] + temp[2] + temp[3];
#elif USE_SSE4_1
			return _mm_cvtss_f32(_mm_dp_ps(m_value, other, 0b11110001));
#elif USE_NEON
			float32x4_t mul = vmulq_f32(m_value, other.m_value);
			return vaddvq_f32(mul);
#else
			return m_values[0] * other.m_values[0] + m_values[1] * other.m_values[1] + m_values[2] * other.m_values[2] +
			       m_values[3] * other.m_values[3];
#endif
		}
		[[nodiscard]] FORCE_INLINE float Dot3Scalar(const Packed other) const
		{
#if USE_WASM_SIMD128
			__f32x4 product = wasm_f32x4_mul(m_value, other.m_value);
			float temp[4];
			wasm_v128_store(temp, product);
			return temp[0] + temp[1] + temp[2];
#elif USE_SSE4_1
			return _mm_cvtss_f32(_mm_dp_ps(m_value, other, 0b01110001));
#elif USE_NEON
			float32x4_t mul = vmulq_f32(m_value, other.m_value);
			mul = vsetq_lane_f32(0, mul, 3);
			return vaddvq_f32(mul);
#else
			return m_values[0] * other.m_values[0] + m_values[1] * other.m_values[1] + m_values[2] * other.m_values[2];
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed GetLengthSquared3() const
		{
			return Dot3(*this);
		}
		[[nodiscard]] FORCE_INLINE Packed GetLengthSquared4() const
		{
			return Dot4(*this);
		}
		[[nodiscard]] FORCE_INLINE float GetLengthSquared3Scalar() const
		{
			return Dot3Scalar(*this);
		}
		[[nodiscard]] FORCE_INLINE float GetLengthSquared4Scalar() const
		{
			return Dot4Scalar(*this);
		}

		[[nodiscard]] FORCE_INLINE Packed GetNormalized4() const
		{
#if USE_WASM_SIMD128
			__f32x4 mul = wasm_f32x4_mul(m_value, m_value);
			float temp[4];
			wasm_v128_store(temp, mul);
			__f32x4 sum = wasm_f32x4_splat(temp[0] + temp[1] + temp[2] + temp[3]);
			return wasm_f32x4_div(m_value, wasm_f32x4_sqrt(sum));
#elif USE_SSE4_1
			return _mm_div_ps(m_value, _mm_sqrt_ps(_mm_dp_ps(m_value, m_value, 0xff)));
#elif USE_NEON
			float32x4_t mul = vmulq_f32(m_value, m_value);
			float32x4_t sum = vdupq_n_f32(vaddvq_f32(mul));
			return vdivq_f32(m_value, vsqrtq_f32(sum));
#else
			return *this / Math::Sqrt(GetLengthSquared4Scalar());
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed GetNormalized3() const
		{
#if USE_WASM_SIMD128
			__f32x4 mul = wasm_f32x4_mul(m_value, m_value);
			float temp[4];
			wasm_v128_store(temp, mul);
			__f32x4 sum = wasm_f32x4_splat(temp[0] + temp[1] + temp[2]);
			return wasm_f32x4_div(m_value, wasm_f32x4_sqrt(sum));
#elif USE_SSE4_1
			return _mm_div_ps(m_value, _mm_sqrt_ps(_mm_dp_ps(m_value, m_value, 0x7f)));
#elif USE_NEON
			float32x4_t mul = vmulq_f32(m_value, m_value);
			mul = vsetq_lane_f32(0, mul, 3);
			float32x4_t sum = vdupq_n_f32(vaddvq_f32(mul));
			return vdivq_f32(m_value, vsqrtq_f32(sum));
#else
			return *this / Math::Sqrt(GetLengthSquared3Scalar());
#endif
		}

		[[nodiscard]] FORCE_INLINE int GetMask() const noexcept
		{
#if USE_WASM_SIMD128
			return (int)(wasm_i32x4_bitmask(m_value) & 0xff);
#elif USE_SSE
			return _mm_movemask_ps(m_value);
#elif USE_NEON
			uint32x4_t mask = vreinterpretq_u32_f32(m_value);
			uint32_t result = (vgetq_lane_u32(mask, 0) >> 31) | ((vgetq_lane_u32(mask, 1) >> 31) << 1) | ((vgetq_lane_u32(mask, 2) >> 31) << 2) |
			                  ((vgetq_lane_u32(mask, 3) >> 31) << 3);
			return result & 0xff;
#endif
		}

		template<uint8 X, uint8 Y, uint8 Z, uint8 W>
		[[nodiscard]] FORCE_INLINE Packed Swizzle() const
		{
			static_assert(X <= 3, "Out of swizzling range");
			static_assert(Y <= 3, "Out of swizzling range");
			static_assert(Z <= 3, "Out of swizzling range");
			static_assert(W <= 3, "Out of swizzling range");

#if USE_SSE
			return _mm_shuffle_ps(m_value, m_value, _MM_SHUFFLE(W, Z, Y, X));
#elif USE_NEON
			if constexpr (X == 0 && Y == 1 && Z == 2 && W == 2)
			{
				return vcombine_f32(vget_low_f32(m_value), vdup_lane_f32(vget_high_f32(m_value), 0));
			}
			else if constexpr (X == 0 && Y == 1 && Z == 3 && W == 3)
			{
				return vcombine_f32(vget_low_f32(m_value), vdup_lane_f32(vget_high_f32(m_value), 1));
			}
			else if constexpr (X == 0 && Y == 1 && Z == 2 && W == 3)
			{
				return m_value;
			}
			else if constexpr (X == 1 && Y == 0 && Z == 3 && W == 2)
			{
				return vcombine_f32(vrev64_f32(vget_low_f32(m_value)), vrev64_f32(vget_high_f32(m_value)));
			}
			else if constexpr (X == 2 && Y == 2 && Z == 1 && W == 0)
			{
				return vcombine_f32(vdup_lane_f32(vget_high_f32(m_value), 0), vrev64_f32(vget_low_f32(m_value)));
			}
			else if constexpr (X == 2 && Y == 3 && Z == 0 && W == 1)
			{
				return vcombine_f32(vget_high_f32(m_value), vget_low_f32(m_value));
			}
			else if constexpr (X == 1 && Y == 2 && Z == 0 && W == 0)
			{
				static const uint8x16_t table{0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03};
				return vreinterpretq_f32_u8(vqtbl1q_u8(vreinterpretq_u8_f32(m_value), table));
			}
			else
			{
				VectorizedType result;
				result = vmovq_n_f32(vgetq_lane_f32(m_value, X & 0b11));
				result = vsetq_lane_f32(vgetq_lane_f32(m_value, Y & 0b11), result, 1);
				result = vsetq_lane_f32(vgetq_lane_f32(m_value, Z & 0b11), result, 2);
				result = vsetq_lane_f32(vgetq_lane_f32(m_value, W & 0b11), result, 3);
				return Packed{result};
			}
#else
			return {m_values[X], m_values[Y], m_values[Z], m_values[W]};
#endif
		}
		[[nodiscard]] FORCE_INLINE Packed yzxw() const
		{
			return Swizzle<1, 2, 0, 3>();
		}
	};

#if USE_AVX
	template<>
	struct TRIVIAL_ABI Packed<float, 8> : public PackedBase<float, __m256, 8>
	{
		inline static constexpr bool IsVectorized = USE_AVX;
		using BaseType = PackedBase<float, __m256, 8>;
		using BaseType::BaseType;

		FORCE_INLINE Packed(const float value) noexcept
			: BaseType(_mm256_set1_ps(value))
		{
		}
		FORCE_INLINE Packed(Math::ZeroType) noexcept
			: BaseType(0.f)
		{
		}
		FORCE_INLINE Packed(const float value[8]) noexcept
			: BaseType(_mm256_set_ps(value[7], value[6], value[5], value[4], value[3], value[2], value[1], value[0]))
		{
		}
		FORCE_INLINE Packed(
			const float value7,
			const float value6,
			const float value5,
			const float value4,
			const float value3,
			const float value2,
			const float value1,
			const float value0
		) noexcept
			: BaseType(_mm256_set_ps(value0, value1, value2, value3, value4, value5, value6, value7))
		{
		}

		FORCE_INLINE void StoreSingle(float& out) noexcept
		{
			_mm_store_ss(&out, _mm256_castps256_ps128(m_value));
		}
		FORCE_INLINE void StoreAll(const FixedArrayView<float, 8> out) noexcept
		{
			_mm256_store_ps(out.GetData(), m_value);
		}

		[[nodiscard]] FORCE_INLINE Packed operator-() const noexcept
		{
			return _mm256_xor_ps(m_value, _mm256_set1_ps(-0.0));
		}
		[[nodiscard]] FORCE_INLINE Packed operator+(const Packed other) const noexcept
		{
			return _mm256_add_ps(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator+=(const Packed other) noexcept
		{
			*this = *this + other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator-(const Packed other) const noexcept
		{
			return _mm256_sub_ps(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator-=(const Packed other) noexcept
		{
			*this = *this - other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator*(const Packed other) const noexcept
		{
			return _mm256_mul_ps(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator*=(const Packed other) noexcept
		{
			*this = *this * other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator/(const Packed other) const noexcept
		{
			return _mm256_div_ps(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator/=(const Packed other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE Packed operator==(const Packed other) const noexcept
		{
			return _mm256_cmp_ps(m_value, other.m_value, _CMP_EQ_OQ);
		}
		[[nodiscard]] FORCE_INLINE Packed operator!=(const Packed other) const noexcept
		{
			return _mm256_cmp_ps(m_value, other.m_value, _CMP_NEQ_OQ);
		}

		[[nodiscard]] FORCE_INLINE Packed operator>(const Packed other) const noexcept
		{
			return _mm256_cmp_ps(m_value, other.m_value, _CMP_GT_OQ);
		}
		[[nodiscard]] FORCE_INLINE Packed operator>=(const Packed other) const noexcept
		{
			return _mm256_cmp_ps(m_value, other.m_value, _CMP_GE_OQ);
		}
		[[nodiscard]] FORCE_INLINE Packed operator<(const Packed other) const noexcept
		{
			return _mm256_cmp_ps(m_value, other.m_value, _CMP_LT_OQ);
		}
		[[nodiscard]] FORCE_INLINE Packed operator<=(const Packed other) const noexcept
		{
			return _mm256_cmp_ps(m_value, other.m_value, _CMP_LE_OQ);
		}
		[[nodiscard]] FORCE_INLINE Packed operator&(const Packed other) const noexcept
		{
			return _mm256_and_ps(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator|(const Packed other) const noexcept
		{
			return _mm256_or_ps(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator^(const Packed other) const noexcept
		{
			return _mm256_xor_ps(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator~() const noexcept
		{
			return _mm256_xor_ps(m_value, _mm256_castsi256_ps(_mm256_set1_epi32(-1)));
		}
		[[nodiscard]] FORCE_INLINE Packed operator!() const noexcept
		{
			__m256 zero = _mm256_setzero_ps();
			__m256 cmp = _mm256_cmp_ps(m_value, zero, _CMP_EQ_OQ);
			return _mm256_xor_ps(cmp, _mm256_set1_ps(1.0f));
		}

		[[nodiscard]] FORCE_INLINE int GetMask() const noexcept
		{
			return _mm256_movemask_ps(m_value);
		}
	};
#endif

#if USE_AVX512
	template<>
	struct TRIVIAL_ABI Packed<float, 16> : public PackedBase<float, __m512, 16>
	{
		inline static constexpr bool IsVectorized = USE_AVX512;
		using BaseType = PackedBase<float, __m512, 16>;
		using BaseType::BaseType;

		FORCE_INLINE Packed(const float value) noexcept
			: BaseType(_mm512_set1_ps(value))
		{
		}
		FORCE_INLINE Packed(Math::ZeroType) noexcept
			: BaseType(0.f)
		{
		}
		FORCE_INLINE Packed(const float value[16]) noexcept
			: BaseType(_mm512_set_ps(
					value[0],
					value[1],
					value[2],
					value[3],
					value[4],
					value[5],
					value[6],
					value[7],
					value[8],
					value[9],
					value[10],
					value[11],
					value[12],
					value[13],
					value[14],
					value[15]
				))
		{
		}

		FORCE_INLINE void StoreSingle(float& out) noexcept
		{
			_mm_storeu_ps(&out, _mm512_castps512_ps128(m_value));
		}
		FORCE_INLINE void StoreAll(const FixedArrayView<float, 16> out) noexcept
		{
			_mm512_store_ps(out.GetData(), m_value);
		}
	};
#endif
}
