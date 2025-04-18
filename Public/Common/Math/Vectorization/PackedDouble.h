#pragma once

#include <Common/Math/Vectorization/Packed.h>

#include <Common/Memory/Containers/FixedArrayView.h>

#if USE_WASM_SIMD128
#include <wasm_simd128.h>
#endif

namespace ngine::Math::Vectorization
{
#if USE_WASM_SIMD128
	template<>
	struct TRIVIAL_ABI Packed<double, 2> : public PackedBase<double, __f64x2, 2>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<double, __f64x2, 2>;
		using BaseType::BaseType;
	};
#elif USE_SSE
	template<>
	struct TRIVIAL_ABI Packed<double, 2> : public PackedBase<double, __m128d, 2>
	{
		inline static constexpr bool IsVectorized = USE_SSE;
		using BaseType = PackedBase<double, __m128d, 2>;
		using BaseType::BaseType;

		FORCE_INLINE Packed(const SetSingleType, const double value) noexcept
			: BaseType(_mm_set_sd(value))
		{
		}
		FORCE_INLINE Packed(const double value) noexcept
			: BaseType(_mm_set1_pd(value))
		{
		}
		FORCE_INLINE Packed(Math::ZeroType) noexcept
			: BaseType(0.0)
		{
		}
		FORCE_INLINE Packed(const double value[2]) noexcept
			: BaseType(_mm_set_pd(value[1], value[0]))
		{
		}

		FORCE_INLINE void StoreSingle(double& out) noexcept
		{
			_mm_store_sd(&out, m_value);
		}
		FORCE_INLINE double GetSingle() noexcept
		{
			return _mm_cvtsd_f64(m_value);
		}

		FORCE_INLINE void StoreAll(const FixedArrayView<double, 2> out) noexcept
		{
			_mm_store_pd(out.GetData(), m_value);
		}

		[[nodiscard]] FORCE_INLINE Packed operator-() const noexcept
		{
			return _mm_xor_pd(m_value, _mm_set1_pd(-0.0));
		}
		[[nodiscard]] FORCE_INLINE Packed operator+(const Packed other) const noexcept
		{
			return _mm_add_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator+=(const Packed other) noexcept
		{
			*this = *this + other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator-(const Packed other) const noexcept
		{
			return _mm_sub_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator-=(const Packed other) noexcept
		{
			*this = *this - other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator*(const Packed other) const noexcept
		{
			return _mm_mul_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator*=(const Packed other) noexcept
		{
			*this = *this * other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator/(const Packed other) const noexcept
		{
			return _mm_div_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator/=(const Packed other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE Packed operator==(const Packed other) const noexcept
		{
			return _mm_cmpeq_pd(m_value, other.m_value);
		}
		[[nodiscard]] FORCE_INLINE Packed operator!=(const Packed other) const noexcept
		{
			return _mm_cmpneq_pd(m_value, other.m_value);
		}

		[[nodiscard]] FORCE_INLINE Packed operator>(const Packed other) const noexcept
		{
			return _mm_cmpgt_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator>=(const Packed other) const noexcept
		{
			return _mm_cmpge_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator<(const Packed other) const noexcept
		{
			return _mm_cmplt_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator<=(const Packed other) const noexcept
		{
			return _mm_cmple_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator&(const Packed other) const noexcept
		{
			return _mm_and_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator|(const Packed other) const noexcept
		{
			return _mm_or_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator^(const Packed other) const noexcept
		{
			return _mm_xor_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed operator~() const noexcept
		{
			return _mm_xor_pd(m_value, _mm_castsi128_pd(_mm_set1_epi32(-1)));
		}
		[[nodiscard]] FORCE_INLINE Packed operator!() const noexcept
		{
			__m128d zero = _mm_setzero_pd();
			__m128d cmp = _mm_cmpeq_pd(m_value, zero);
			return _mm_xor_pd(cmp, _mm_set1_pd(1.0));
		}

		[[nodiscard]] FORCE_INLINE Packed Dot2(const Packed other) const noexcept
		{
#if USE_SSE4_1
			return _mm_dp_pd(m_value, other, 0xff);
#else
			return {m_values[0] * other.m_values[0], m_values[1] * other.m_values[1]};
#endif
		}
		[[nodiscard]] FORCE_INLINE double Dot2Scalar(const Packed other) const noexcept
		{
#if USE_SSE4_1
			return _mm_cvtsd_f64(_mm_dp_pd(m_value, other, 0b00110001));
#else
			return m_values[0] * other.m_values[0] + m_values[1] * other.m_values[1];
#endif
		}

		[[nodiscard]] FORCE_INLINE Packed GetLengthSquared2() const
		{
			return Dot2(*this);
		}
		[[nodiscard]] FORCE_INLINE double GetLengthSquared2Scalar() const
		{
			return Dot2Scalar(*this);
		}

		[[nodiscard]] FORCE_INLINE int GetMask() const noexcept
		{
			return _mm_movemask_pd(m_value);
		}
	};
#elif USE_NEON
	template<>
	struct TRIVIAL_ABI Packed<double, 2> : public PackedBase<double, float64x2_t, 2>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<double, float64x2_t, 2>;
		using BaseType::BaseType;
	};
#endif

#if USE_AVX
	template<>
	struct TRIVIAL_ABI Packed<double, 4> : public PackedBase<double, __m256d, 4>
	{
		inline static constexpr bool IsVectorized = USE_AVX;
		using BaseType = PackedBase<double, __m256d, 4>;
		using BaseType::BaseType;

		FORCE_INLINE Packed(const double value) noexcept
			: BaseType(_mm256_set1_pd(value))
		{
		}
		FORCE_INLINE Packed(Math::ZeroType) noexcept
			: BaseType(0.0)
		{
		}
		FORCE_INLINE Packed(const double value3, const double value2, const double value1, const double value0) noexcept
			: BaseType(_mm256_set_pd(value0, value1, value2, value3))
		{
		}
		FORCE_INLINE Packed(const double value[4]) noexcept
			: BaseType(_mm256_set_pd(value[0], value[1], value[2], value[3]))
		{
		}

		FORCE_INLINE void StoreAll(const FixedArrayView<double, 4> out) noexcept
		{
			_mm256_store_pd(out.GetData(), m_value);
		}

		[[nodiscard]] FORCE_INLINE Packed operator-() const noexcept
		{
			return _mm256_xor_pd(m_value, _mm256_set1_pd(-0.0));
		}
		[[nodiscard]] FORCE_INLINE Packed operator+(const Packed other) const noexcept
		{
			return _mm256_add_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator+=(const Packed other) noexcept
		{
			*this = *this + other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator-(const Packed other) const noexcept
		{
			return _mm256_sub_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator-=(const Packed other) noexcept
		{
			*this = *this - other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator*(const Packed other) const noexcept
		{
			return _mm256_mul_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator*=(const Packed other) noexcept
		{
			*this = *this * other;
			return *this;
		}
		[[nodiscard]] FORCE_INLINE Packed operator/(const Packed other) const noexcept
		{
			return _mm256_div_pd(m_value, other);
		}
		[[nodiscard]] FORCE_INLINE Packed& operator/=(const Packed other) noexcept
		{
			*this = *this / other;
			return *this;
		}

		[[nodiscard]] FORCE_INLINE Packed operator==(const Packed other) const noexcept
		{
			return _mm256_cmp_pd(m_value, other.m_value, _CMP_EQ_OQ);
		}
		[[nodiscard]] FORCE_INLINE Packed operator!=(const Packed other) const noexcept
		{
			return _mm256_cmp_pd(m_value, other.m_value, _CMP_NEQ_OQ);
		}

		[[nodiscard]] FORCE_INLINE Packed operator>(const Packed other) const noexcept
		{
			return _mm256_cmp_pd(m_value, other.m_value, _CMP_GT_OQ);
		}
		[[nodiscard]] FORCE_INLINE Packed operator>=(const Packed other) const noexcept
		{
			return _mm256_cmp_pd(m_value, other.m_value, _CMP_GE_OQ);
		}
		[[nodiscard]] FORCE_INLINE Packed operator<(const Packed other) const noexcept
		{
			return _mm256_cmp_pd(m_value, other.m_value, _CMP_LT_OQ);
		}
		[[nodiscard]] FORCE_INLINE Packed operator<=(const Packed other) const noexcept
		{
			return _mm256_cmp_pd(m_value, other.m_value, _CMP_LE_OQ);
		}
		[[nodiscard]] FORCE_INLINE Packed operator!() const noexcept
		{
			__m256d zero = _mm256_setzero_pd();
			__m256d cmp = _mm256_cmp_pd(m_value, zero, _CMP_EQ_OQ);
			return _mm256_xor_pd(cmp, _mm256_set1_pd(1.0));
		}

		[[nodiscard]] FORCE_INLINE int GetMask() const noexcept
		{
			return _mm256_movemask_pd(m_value);
		}
	};
#endif

#if USE_AVX512
	template<>
	struct TRIVIAL_ABI Packed<double, 8> : public PackedBase<double, __m512d, 8>
	{
		inline static constexpr bool IsVectorized = USE_AVX512;
		using BaseType = PackedBase<double, __m512d, 8>;
		using BaseType::BaseType;

		FORCE_INLINE Packed(const double value) noexcept
			: m_value(_mm512_set1_pd(value))
		{
		}
		FORCE_INLINE Packed(Math::ZeroType) noexcept
			: BaseType(0.0)
		{
		}
		FORCE_INLINE Packed(const double value[8]) noexcept
			: m_value(_mm512_set_pd(value[0], value[1], value[2], value[3], value[4], value[5], value[6], value[7]))
		{
		}

		FORCE_INLINE void StoreSingle(double& out) noexcept
		{
			_mm_storeu_pd(&out, _mm512_castps512_pd128(m_value));
		}
		FORCE_INLINE void StoreAll(double* pOut) noexcept
		{
			_mm512_store_pd(pOut, m_value);
		}
	};
#endif
}
