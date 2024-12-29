#pragma once

#include <Common/Math/MathAssert.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/CanWrite.h>
#include <Common/Platform/TrivialABI.h>

#include <Common/Math/Vector2.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TRectangleEdges
	{
		inline static constexpr Guid TypeGuid = "0be4adff-8f65-4341-9328-765ee6b9b7ee"_guid;

		FORCE_INLINE TRectangleEdges() = default;
		FORCE_INLINE TRectangleEdges(const T value)
			: m_top(value)
			, m_right(value)
			, m_bottom(value)
			, m_left(value)
		{
		}
		FORCE_INLINE TRectangleEdges(const T top, const T right, const T bottom, const T left)
			: m_top(top)
			, m_right(right)
			, m_bottom(bottom)
			, m_left(left)
		{
		}
		FORCE_INLINE TRectangleEdges(const T vertical, const T horizontal)
			: m_top(vertical)
			, m_right(horizontal)
			, m_bottom(vertical)
			, m_left(horizontal)
		{
		}

		using Vector2Type = TVector2<T>;

		[[nodiscard]] FORCE_INLINE Vector2Type GetSum() const
		{
			return Vector2Type{m_left, m_top} + Vector2Type{m_right, m_bottom};
		}

		[[nodiscard]] FORCE_INLINE TRectangleEdges operator-() const
		{
			return {-m_top, -m_right, -m_bottom, -m_left};
		}

		[[nodiscard]] FORCE_INLINE bool operator==(const TRectangleEdges& other) const
		{
			return (m_left == other.m_left) & (m_top == other.m_top) & (m_right == other.m_right) & (m_bottom == other.m_bottom);
		}
		[[nodiscard]] FORCE_INLINE bool operator!=(const TRectangleEdges& other) const
		{
			return !operator==(other);
		}

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T& operator[](const uint8 index) noexcept
		{
			MathExpect(index < 4);
			return *(&m_top + index);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T operator[](const uint8 index) const noexcept
		{
			MathExpect(index < 4);
			return *(&m_top + index);
		}

		template<typename... Args>
		EnableIf<Serialization::Internal::CanRead<T, Args...>, bool> Serialize(const Serialization::Reader reader, Args&... args);
		template<typename... Args>
		EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool> Serialize(Serialization::Writer writer, Args&... args) const;

		T m_top;
		T m_right;
		T m_bottom;
		T m_left;
	};

	using RectangleEdgesf = TRectangleEdges<float>;
	using RectangleEdgesi = TRectangleEdges<int32>;
}
