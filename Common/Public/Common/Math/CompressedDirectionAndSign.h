#pragma once

#include <Common/Math/Vector4.h>
#include <Common/Math/Vector4/Truncate.h>
#include <Common/Math/Vector4/Min.h>
#include <Common/Math/Vector4/Max.h>
#include <Common/Math/Vector3.h>
#include <Common/Math/Vector3/Truncate.h>
#include <Common/Math/Vector3/Min.h>
#include <Common/Math/Vector3/Max.h>
#include <Common/Math/Clamp.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	//! Helper class for compressing and decompressing a 3D direction and a sign
	//! Equivalent to the RGB10A2 format
	struct TRIVIAL_ABI CompressedDirectionAndSign
	{
		inline static constexpr uint16 AxisBitCount = 10;
		inline static constexpr uint16 SignBitCount = 2;
		inline static constexpr uint16 MaximumAxisLength = (1 << AxisBitCount) - 1;
		inline static constexpr uint16 MaximumSignLength = (1 << SignBitCount) - 1;

		FORCE_INLINE CompressedDirectionAndSign() = default;
		FORCE_INLINE CompressedDirectionAndSign(const uint32 packedData)
			: m_packedData(packedData)
		{
		}
		FORCE_INLINE CompressedDirectionAndSign(const Math::Vector4f in)
		{
			constexpr float halfMaximumAxisLength = (float)MaximumAxisLength / 2.f;
			constexpr float halfMaximumSignLength = (float)MaximumSignLength / 2.f;

			const Math::Vector4f halfValues =
				Math::Vector4f(halfMaximumAxisLength, halfMaximumAxisLength, halfMaximumAxisLength, halfMaximumSignLength);
			const Math::Vector4i truncated = Math::Truncate<Math::Vector4i>(in * halfValues + halfValues);
			const Math::Vector4i compressedValue = Math::Clamp(
				truncated,
				Math::Vector4i(Math::Zero),
				Math::Vector4i(MaximumAxisLength, MaximumAxisLength, MaximumAxisLength, MaximumSignLength)
			);
			x = (uint32)compressedValue.x;
			y = (uint32)compressedValue.y;
			z = (uint32)compressedValue.z;
			w = (uint32)compressedValue.w;
		}
		FORCE_INLINE CompressedDirectionAndSign(const Math::Vector3f in, const float sign)
			: CompressedDirectionAndSign({in.x, in.y, in.z, sign})
		{
		}
		FORCE_INLINE CompressedDirectionAndSign(const Math::Vector3f in)
		{
			constexpr float halfMaximumAxisLength = (float)MaximumAxisLength / 2.f;

			const Math::Vector3f halfValues = Math::Vector3f(halfMaximumAxisLength, halfMaximumAxisLength, halfMaximumAxisLength);
			const Math::Vector3i truncated = Math::Truncate<Math::Vector3i>(in * halfValues + halfValues);
			const Math::Vector3i compressedValue =
				Math::Clamp(truncated, Math::Vector3i(Math::Zero), Math::Vector3i(MaximumAxisLength, MaximumAxisLength, MaximumAxisLength));
			x = (uint32)compressedValue.x;
			y = (uint32)compressedValue.y;
			z = (uint32)compressedValue.z;
			w = 3;
		}

		[[nodiscard]] FORCE_INLINE operator Math::Vector4f() const
		{
			const Math::Vector4i compressedVector = (Math::Vector4i(m_packedData) >>
			                                         Math::Vector4i(AxisBitCount * 0, AxisBitCount * 1, AxisBitCount * 2, AxisBitCount * 3)) &
			                                        Math::Vector4i(MaximumAxisLength, MaximumAxisLength, MaximumAxisLength, MaximumSignLength);

			constexpr float inverseAxisDivisor = 1.0f / float(MaximumAxisLength);
			constexpr float inverseSignDivisor = 1.0f / float(MaximumSignLength);

			const Math::Vector4f packed = Math::Vector4f(compressedVector) *
			                              Math::Vector4f(inverseAxisDivisor, inverseAxisDivisor, inverseAxisDivisor, inverseSignDivisor);
			return Math::Vector4f(-1.f) + packed * Math::Vector4f(2.f);
		}

		[[nodiscard]] FORCE_INLINE operator Math::Vector3f() const
		{
			const Math::Vector3i compressedVector = (Math::Vector3i(m_packedData) >>
			                                         Math::Vector3i(AxisBitCount * 0, AxisBitCount * 1, AxisBitCount * 2)) &
			                                        Math::Vector3i(MaximumAxisLength, MaximumAxisLength, MaximumAxisLength);

			constexpr float inverseAxisDivisor = 1.0f / float(MaximumAxisLength);

			const Math::Vector3f packed = Math::Vector3f(compressedVector) *
			                              Math::Vector3f(inverseAxisDivisor, inverseAxisDivisor, inverseAxisDivisor);
			return Math::Vector3f(-1.f) + packed * Math::Vector3f(2.f);
		}
	protected:
		union
		{
			struct
			{
				uint32 x : 10;
				uint32 y : 10;
				uint32 z : 10;
				uint32 w : 2;
			};
			uint32 m_packedData = 0;
		};
	};
}
