#pragma once

#include <Common/Math/Primitives/ForwardDeclarations/Plane.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename VectorType>
	struct TRIVIAL_ABI TPlane
	{
		TPlane(const VectorType position, const Vector3f normal)
			: m_position(position)
			, m_normal(normal)
		{
		}

		[[nodiscard]] FORCE_INLINE VectorType GetPosition() const
		{
			return m_position;
		}
		[[nodiscard]] FORCE_INLINE Vector3f GetNormal() const
		{
			return m_normal;
		}
	protected:
		VectorType m_position;
		Vector3f m_normal;
	};
}
