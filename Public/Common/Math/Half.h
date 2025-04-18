#pragma once

#include <Common/Math/NumericLimits.h>

#include <Common/Math/Vectorization/PackedInt32.h>
#include <Common/Math/Vectorization/PackedFloat.h>
#include <Common/Math/Vectorization/Min.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine
{
#define HAS_HALF_TYPE (COMPILER_CLANG || COMPILER_GCC) && !PLATFORM_WINDOWS && !PLATFORM_EMSCRIPTEN && !PLATFORM_LINUX

#if HAS_HALF_TYPE
	using half = _Float16;
#else
	struct TRIVIAL_ABI half
	{
	protected:
#if USE_SSE
		using PackedInt32 = Math::Vectorization::Packed<int32, 4>;
		using PackedFloat = Math::Vectorization::Packed<float, 4>;

		// Half <-> Float implementation is based on:
		// http://fgiesen.wordpress.com/2012/03/28/half-to-float-done-quic/.
		[[nodiscard]] static FORCE_INLINE PackedInt32 FloatToHalf(const PackedFloat value)
		{
			const PackedInt32 mask_sign = 0x80000000u;
			const PackedInt32 mask_round = ~0xfffu;
			const PackedInt32 f32infty = 255 << 23;
			const PackedFloat magic = _mm_castsi128_ps(_mm_set1_epi32(15 << 23));
			const PackedInt32 nanbit = 0x200;
			const PackedInt32 infty_as_fp16 = 0x7c00;
			const PackedFloat clamp = _mm_castsi128_ps(_mm_set1_epi32((31 << 23) - 0x1000));

			const PackedFloat msign = _mm_castsi128_ps(mask_sign);
			const PackedFloat justsign = msign & value;
			const PackedFloat absf = value ^ justsign;
			const PackedFloat mround = _mm_castsi128_ps(mask_round);
			const PackedInt32 absf_int = _mm_castps_si128(absf);
			const PackedInt32 b_isnan = absf_int > f32infty;
			const PackedInt32 b_isnormal = f32infty > PackedInt32{_mm_castps_si128(absf)};
			const PackedInt32 inf_or_nan = ((b_isnan & nanbit) | infty_as_fp16);
			const PackedFloat fnosticky = (absf & mround);
			const PackedFloat scaled = (fnosticky * magic);
			// Logically, we want PMINSD on "biased", but this should gen better code
			const PackedFloat clamped = Math::Min(scaled, clamp);
			const PackedInt32 biased = (PackedInt32(_mm_castps_si128(clamped)) - PackedInt32(_mm_castps_si128(mround)));
			const PackedInt32 shifted = _mm_srli_epi32(biased, 13);
			const PackedInt32 normal = (shifted & b_isnormal);
			const PackedInt32 not_normal = _mm_andnot_si128(b_isnormal, inf_or_nan);
			const PackedInt32 joined = (normal | not_normal);

			const PackedInt32 sign_shift = _mm_srli_epi32(_mm_castps_si128(justsign), 16);
			return (joined | sign_shift);
		}

		[[nodiscard]] static FORCE_INLINE PackedFloat HalfToFloat(const PackedInt32 value)
		{
			const PackedInt32 mask_nosign = _mm_set1_epi32(0x7fff);
			const PackedFloat magic = _mm_castsi128_ps(_mm_set1_epi32((254 - 15) << 23));
			const PackedInt32 was_infnan = _mm_set1_epi32(0x7bff);
			const PackedFloat exp_infnan = _mm_castsi128_ps(_mm_set1_epi32(255 << 23));

			const PackedInt32 expmant = (mask_nosign & value);
			const PackedInt32 shifted = _mm_slli_epi32(expmant, 13);
			const PackedFloat scaled = (PackedFloat(_mm_castsi128_ps(shifted)) * magic);
			const PackedInt32 b_wasinfnan = (expmant > was_infnan);
			const PackedInt32 sign = _mm_slli_epi32((value ^ expmant), 16);
			const PackedFloat infnanexp = (PackedFloat(_mm_castsi128_ps(b_wasinfnan)) & exp_infnan);
			const PackedFloat sign_inf = (PackedFloat(_mm_castsi128_ps(sign)) | infnanexp);
			return (scaled | sign_inf);
		}
#endif

		// Half <-> Float implementation is based on:
		// http://fgiesen.wordpress.com/2012/03/28/half-to-float-done-quic/.
		[[nodiscard]] FORCE_INLINE static uint16 FloatToHalf(const float value)
		{
#if USE_SSE
			return static_cast<uint16>(FloatToHalf(PackedFloat{value}).GetSingle());
#else
			constexpr uint32_t f32infty = 255 << 23;
			constexpr uint32_t f16infty = 31 << 23;
			const union
			{
				uint32_t u;
				float f;
			} magic = {15 << 23};
			const uint32_t sign_mask = 0x80000000u;
			const uint32_t round_mask = ~0x00000fffu;

			const union
			{
				float f;
				uint32_t u;
			} f = {value};
			const uint32_t sign = f.u & sign_mask;
			const uint32_t f_nosign = f.u & ~sign_mask;

			if (f_nosign >= f32infty)
			{ // Inf or NaN (all exponent bits set)
				// NaN->qNaN and Inf->Inf
				const uint32_t result = ((f_nosign > f32infty) ? 0x7e00 : 0x7c00) | (sign >> 16);
				return static_cast<uint16_t>(result);
			}
			else
			{ // (De)normalized number or zero
				const union
				{
					uint32_t u;
					float f;
				} rounded = {f_nosign & round_mask};
				const union
				{
					float f;
					uint32_t u;
				} exp = {rounded.f * magic.f};
				const uint32_t re_rounded = exp.u - round_mask;
				// Clamp to signed infinity if overflowed
				const uint32_t result = ((re_rounded > f16infty ? f16infty : re_rounded) >> 13) | (sign >> 16);
				return static_cast<uint16_t>(result);
			}
#endif
		}
	public:
		half() = default;
		FORCE_INLINE half(const float value)
			: m_value(FloatToHalf(value))
		{
		}
		FORCE_INLINE half& operator=(const float value)
		{
			m_value = FloatToHalf(value);
			return *this;
		}

		[[nodiscard]] FORCE_INLINE explicit operator float() const
		{
#if USE_SSE
			return HalfToFloat(PackedInt32{m_value}).GetSingle();
#else
			const union
			{
				uint32_t u;
				float f;
			} magic = {(254 - 15) << 23};
			const union
			{
				uint32_t u;
				float f;
			} infnan = {(127 + 16) << 23};

			const uint16 value = m_value;
			const uint32_t sign = value & 0x8000;
			const union
			{
				int32_t u;
				float f;
			} exp_mant = {(value & 0x7fff) << 13};
			const union
			{
				float f;
				uint32_t u;
			} adjust = {exp_mant.f * magic.f};
			// Make sure Inf/NaN survive
			const union
			{
				uint32_t u;
				float f;
			} result = {(adjust.f >= infnan.f ? (adjust.u | 255 << 23) : adjust.u) | (sign << 16)};
			return result.f;
#endif
		}
		[[nodiscard]] FORCE_INLINE constexpr half operator-() const
		{
			uint16 value = m_value;
			value ^= 0x8000;
			return half{value};
		}
	protected:
		constexpr half(const uint16 value)
			: m_value(value)
		{
		}
		uint16 m_value;
	};
#endif

	namespace Math
	{
		template<>
		struct NumericLimits<half>
		{
			inline static half MinPositive = 0.000060975552f;
			inline static half Max = 65504.f;
			inline static half Min = -Max;
			inline static half Epsilon = half(0.1f);
			inline static constexpr bool IsUnsigned = false;
		};
	}
}
