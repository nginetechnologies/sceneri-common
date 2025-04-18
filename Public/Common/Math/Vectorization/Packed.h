#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Platform/ForceInline.h>
#include <Common/Platform/TrivialABI.h>
#include <Common/Math/MathAssert.h>
#include <Common/TypeTraits/Select.h>
#include <Common/TypeTraits/EnableIf.h>
#include <Common/TypeTraits/IsSame.h>

#include <Common/Math/Vectorization/NativeTypes.h>

namespace ngine::Math::Vectorization
{
	enum class SetSingleType
	{
		SetSingle
	};
	inline static constexpr SetSingleType SetSingle = SetSingleType::SetSingle;
	enum class LoadUnalignedType
	{
		LoadUnaligned
	};
	inline static constexpr LoadUnalignedType LoadUnaligned = LoadUnalignedType::LoadUnaligned;
	enum class LoadAlignedType
	{
		LoadAligned
	};
	inline static constexpr LoadAlignedType LoadAligned = LoadAlignedType::LoadAligned;

	template<typename Type, typename VectorizedType_, size Count>
	struct TRIVIAL_ABI PackedBase
	{
		using UnitType = Type;
		using VectorizedType = VectorizedType_;

		FORCE_INLINE PackedBase() = default;
		FORCE_INLINE PackedBase(const PackedBase&) = default;
		FORCE_INLINE PackedBase(PackedBase&&) = default;
		FORCE_INLINE PackedBase& operator=(const PackedBase&) = default;
		FORCE_INLINE PackedBase& operator=(PackedBase&&) = default;

		template<
			typename ThisType = Type,
			typename ThisVectorizedType = VectorizedType,
			typename = EnableIf<!TypeTraits::IsSame<ThisType, ThisVectorizedType>>>
		FORCE_INLINE constexpr PackedBase(const VectorizedType value) noexcept
			: m_value(value)
		{
		}
		FORCE_INLINE PackedBase(const SetSingleType, const Type value) noexcept
		{
			m_values[0] = value;
		}
		FORCE_INLINE constexpr PackedBase(const Type value) noexcept
		{
			for (size i = 0; i < Count; ++i)
			{
				m_values[i] = value;
			}
		}
		template<typename... Args, size RequiredCount = Count, typename = EnableIf<sizeof...(Args) == RequiredCount>>
		FORCE_INLINE constexpr PackedBase(const Args... args)
			: m_values{(Type)args...}
		{
		}

		FORCE_INLINE constexpr PackedBase(const Type values[Count]) noexcept
		{
			for (size i = 0; i < Count; ++i)
			{
				m_values[i] = values[i];
			}
		}

		[[nodiscard]] FORCE_INLINE constexpr operator VectorizedType() const noexcept
		{
			return m_value;
		}

		[[nodiscard]] FORCE_INLINE constexpr Type& operator[](const uint8 index) noexcept
		{
			return m_values[index];
		}
		[[nodiscard]] FORCE_INLINE constexpr Type operator[](const uint8 index) const noexcept
		{
			return m_values[index];
		}

		union
		{
			VectorizedType m_value;
			Type m_values[Count];
		};
	};

	template<typename T, uint8 Count>
	struct TRIVIAL_ABI Packed : public PackedBase<T, T, Count>
	{
		inline static constexpr bool IsVectorized = false;
		using BaseType = PackedBase<T, T, Count>;
		using BaseType::BaseType;
		using BaseType::operator=;
	};

	template<typename Type, uint8 Count>
	[[nodiscard]] FORCE_INLINE constexpr Packed<Type, Count> operator~(const Packed<Type, Count> value)
	{
		return value ^ (value == value);
	}

	namespace Internal
	{
		[[nodiscard]] FORCE_INLINE constexpr uint32 GetShuffleMask(const uint8 fp3, const uint8 fp2, const uint8 fp1, const uint8 fp0)
		{
			return uint32((fp3 << 6) | (fp2 << 4) | (fp1 << 2) | fp0);
		}
	}
}

#include <Common/Math/Vectorization/PackedDouble.h>
#include <Common/Math/Vectorization/PackedFloat.h>
#include <Common/Math/Vectorization/PackedInt64.h>
#include <Common/Math/Vectorization/PackedInt32.h>
#include <Common/Math/Vectorization/PackedInt16.h>
#include <Common/Math/Vectorization/PackedInt8.h>
