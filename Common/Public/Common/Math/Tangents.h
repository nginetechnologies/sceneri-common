#pragma once

#include <Common/Math/Vector3.h>
#include <Common/Math/Vector4.h>
#include <Common/Math/Vector3/SignNonZero.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	struct TRIVIAL_ABI CompressedTangent
	{
		FORCE_INLINE CompressedTangent() = default;

		FORCE_INLINE CompressedTangent(const typename Math::Vector3f tangent, const float orientationSign)
			: m_tangent(tangent.x, tangent.y, tangent.z, orientationSign)
		{
		}

		FORCE_INLINE
		CompressedTangent(const typename Math::Vector3f normal, const typename Math::Vector3f tangent, const typename Math::Vector3f bitangent)
			: m_tangent(tangent.x, tangent.y, tangent.z, CalculateBitangentOrientationSign(normal, tangent, bitangent))
		{
		}

		FORCE_INLINE CompressedTangent(const Math::Vector4f tangent)
			: m_tangent(tangent)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS Math::Vector3f CalculateBitangent(const Math::Vector3f normal) const
		{
			return normal.Cross(Math::Vector3f(m_tangent.x, m_tangent.y, m_tangent.z)) * m_tangent.w;
		}

		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS static float CalculateBitangentOrientationSign(
			const typename Math::Vector3f normal, const typename Math::Vector3f tangent, const typename Math::Vector3f bitangent
		)
		{
			return Math::SignNonZero(tangent.Dot(bitangent.Cross(normal)));
		}

		Math::Vector4f m_tangent;
	};

	struct UncompressedTangents
	{
		[[nodiscard]] FORCE_INLINE PURE_NOSTATICS CompressedTangent GetCompressed() const
		{
			return CompressedTangent(m_normal, m_tangent, m_bitangent);
		}

		FORCE_INLINE void SetBitangent(const float orientationSign)
		{
			m_bitangent = CompressedTangent(m_tangent, orientationSign).CalculateBitangent(m_normal);
		}

		Math::Vector3f m_normal;
		Math::Vector3f m_tangent;
		Math::Vector3f m_bitangent;
	};
}
