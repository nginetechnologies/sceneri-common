#pragma once

#include "../RectangleEdges.h"

#include <Common/Serialization/Reader.h>
#include <Common/Serialization/Writer.h>

namespace ngine::Math
{
	template<typename T>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanRead<T, Args...>, bool>
	TRectangleEdges<T>::Serialize(const Serialization::Reader reader, Args&... args)
	{
		if (!reader.IsObject())
		{
			if (reader.SerializeInPlace(m_left, args...))
			{
				m_top = m_left;
				m_right = m_left;
				m_bottom = m_left;
				return true;
			}
		}

		reader.Serialize("left", m_left, args...);
		reader.Serialize("top", m_top, args...);
		reader.Serialize("right", m_right, args...);
		reader.Serialize("bottom", m_bottom, args...);
		return true;
	}

	template<typename T>
	template<typename... Args>
	inline EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool>
	TRectangleEdges<T>::Serialize(Serialization::Writer writer, Args&... args) const
	{
		if (m_left == m_top && m_top == m_right && m_right == m_bottom)
		{
			return writer.SerializeInPlace(m_left, args...);
		}

		bool wroteAny = true;
		wroteAny |= writer.Serialize("left", m_left, args...);
		wroteAny |= writer.Serialize("top", m_top, args...);
		wroteAny |= writer.Serialize("right", m_right, args...);
		wroteAny |= writer.Serialize("bottom", m_bottom, args...);
		return wroteAny;
	}
}
