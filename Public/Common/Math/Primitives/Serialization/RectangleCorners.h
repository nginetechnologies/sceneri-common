#pragma once

#include "../RectangleCorners.h"

#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/CanWrite.h>

namespace ngine::Math
{
	template<typename T>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanRead<T, Args...>, bool>
	TRectangleCorners<T>::Serialize(const Serialization::Reader reader, Args&... args)
	{
		if (!reader.IsObject())
		{
			if (reader.SerializeInPlace(m_topLeft, args...))
			{
				m_topRight = m_topLeft;
				m_bottomLeft = m_topLeft;
				m_bottomRight = m_topLeft;
				return true;
			}
		}

		reader.Serialize("top_left", m_topLeft, args...);
		reader.Serialize("top_right", m_topRight, args...);
		reader.Serialize("bottom_left", m_bottomLeft, args...);
		reader.Serialize("bottom_right", m_bottomRight, args...);
		return true;
	}

	template<typename T>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool>
	TRectangleCorners<T>::Serialize(Serialization::Writer writer, Args&... args) const
	{
		if (m_topLeft == m_topRight && m_topRight == m_bottomLeft && m_bottomLeft == m_bottomRight)
		{
			return writer.SerializeInPlace(m_topLeft, args...);
		}

		bool wroteAny = true;
		wroteAny |= writer.Serialize("top_left", m_topLeft, args...);
		wroteAny |= writer.Serialize("top_right", m_topRight, args...);
		wroteAny |= writer.Serialize("bottom_left", m_bottomLeft, args...);
		wroteAny |= writer.Serialize("bottom_right", m_bottomRight, args...);
		return wroteAny;
	}
}
