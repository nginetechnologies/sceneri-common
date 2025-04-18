#pragma once

#include <Common/Math/Hash.h>

namespace ngine::Math
{
	template<typename Type>
	struct HashedObject
	{
		using is_transparent = void;

		constexpr HashedObject() = default;
		constexpr HashedObject(const Type& object)
			: m_hash(Math::Hash(object))
		{
		}
		constexpr HashedObject& operator=(const Type& object)
		{
			m_hash = Math::Hash(object);
			return *this;
		}

		[[nodiscard]] FORCE_INLINE constexpr operator size() const
		{
			return m_hash;
		}

		struct Hash
		{
			using is_transparent = void;

			[[nodiscard]] FORCE_INLINE constexpr size operator()(const HashedObject<Type> value) const noexcept
			{
				return value;
			}
			[[nodiscard]] FORCE_INLINE constexpr size operator()(const Type& value) const noexcept
			{
				return HashedObject(value);
			}
			[[nodiscard]] FORCE_INLINE constexpr size operator()(Type&& value) const noexcept
			{
				return HashedObject(value);
			}
		};

		struct EqualityCheck
		{
			using is_transparent = void;

			template<typename LeftType, typename RightType>
			constexpr bool operator()(const LeftType& leftType, const RightType& rightType) const
			{
				return HashedObject(leftType) == HashedObject(rightType);
			}
		};

		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const Type& other) const
		{
			return m_hash == Math::Hash(other);
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator==(const HashedObject other) const
		{
			return m_hash == other.m_hash;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator!=(const HashedObject other) const
		{
			return m_hash != other.m_hash;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>(const HashedObject other) const
		{
			return m_hash > other.m_hash;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator>=(const HashedObject other) const
		{
			return m_hash >= other.m_hash;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<(const HashedObject other) const
		{
			return m_hash < other.m_hash;
		}
		[[nodiscard]] FORCE_INLINE constexpr bool operator<=(const HashedObject other) const
		{
			return m_hash <= other.m_hash;
		}
	protected:
		size m_hash{0};
	};
}
