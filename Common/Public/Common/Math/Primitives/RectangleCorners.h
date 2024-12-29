#pragma once

#include <Common/Math/MathAssert.h>
#include <Common/Serialization/ForwardDeclarations/Reader.h>
#include <Common/Serialization/ForwardDeclarations/Writer.h>
#include <Common/Serialization/CanRead.h>
#include <Common/Serialization/CanWrite.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TRectangleCorners
	{
		inline static constexpr Guid TypeGuid = "4f811179-e7e5-4a12-abbb-5e368bb8bc37"_guid;

		TRectangleCorners()
		{
		}
		TRectangleCorners(const T value)
			: m_topLeft(value)
			, m_topRight(value)
			, m_bottomRight(value)
			, m_bottomLeft(value)
		{
		}
		TRectangleCorners& operator=(const T value)
		{
			m_topLeft = value;
			m_topRight = value;
			m_bottomRight = value;
			m_bottomLeft = value;
			return *this;
		}

		TRectangleCorners(const T topLeft, const T topRight, const T bottomRight, const T bottomLeft)
			: m_topLeft(topLeft)
			, m_topRight(topRight)
			, m_bottomRight(bottomRight)
			, m_bottomLeft(bottomLeft)
		{
		}

		TRectangleCorners(const TRectangleCorners&) = default;
		TRectangleCorners& operator=(const TRectangleCorners&) = default;

		[[nodiscard]] FORCE_INLINE bool operator==(const TRectangleCorners& other) const
		{
			return (m_topLeft == other.m_topLeft) & (m_topRight == other.m_topRight) & (m_bottomRight == other.m_bottomRight) &
			       (m_bottomLeft == other.m_bottomLeft);
		}
		[[nodiscard]] FORCE_INLINE bool operator!=(const TRectangleCorners& other) const
		{
			return !operator==(other);
		}

		template<typename... Args>
		EnableIf<Serialization::Internal::CanRead<T, Args...>, bool> Serialize(const Serialization::Reader reader, Args&... args);
		template<typename... Args>
		EnableIf<Serialization::Internal::CanWrite<T, Args...>, bool> Serialize(Serialization::Writer writer, Args&... args) const;

		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T& operator[](const uint8 index) noexcept
		{
			MathExpect(index < 4);
			return *(&m_topLeft + index);
		}
		[[nodiscard]] FORCE_INLINE PURE_LOCALS_AND_POINTERS constexpr T operator[](const uint8 index) const noexcept
		{
			MathExpect(index < 4);
			return *(&m_topLeft + index);
		}

		T m_topLeft;
		T m_topRight;
		T m_bottomRight;
		T m_bottomLeft;
	};

	using RectangleCornersf = TRectangleCorners<float>;
	using RectangleCornersi = TRectangleCorners<int32>;
}
