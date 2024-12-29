#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::Memory
{
	enum class ReserveType : uint8
	{
		Reserve
	};
	inline static constexpr ReserveType Reserve = ReserveType::Reserve;

	enum class ConstructWithSizeType : uint8
	{
		ConstructWithSize
	};
	inline static constexpr ConstructWithSizeType ConstructWithSize = ConstructWithSizeType::ConstructWithSize;

	enum class DefaultConstructType : uint8
	{
		DefaultConstruct
	};
	inline static constexpr DefaultConstructType DefaultConstruct = DefaultConstructType::DefaultConstruct;

	enum class UninitializedType : uint8
	{
		Uninitialized
	};
	inline static constexpr UninitializedType Uninitialized = UninitializedType::Uninitialized;

	enum class ZeroedType : uint8
	{
		Zeroed
	};
	inline static constexpr ZeroedType Zeroed = ZeroedType::Zeroed;

	enum class InitializeAllType : uint8
	{
		InitializeAll
	};
	inline static constexpr InitializeAllType InitializeAll = InitializeAllType::InitializeAll;

	enum class SetAllType : uint8
	{
		SetAll
	};
	inline static constexpr SetAllType SetAll = SetAllType::SetAll;

	enum class ConstructInPlaceType : uint8
	{
		ConstructInPlace
	};
	inline static constexpr ConstructInPlaceType ConstructInPlace = ConstructInPlaceType::ConstructInPlace;

	namespace Internal
	{
		template<typename Type>
		struct DefaultEqualityCheck
		{
			using is_transparent = void;

			template<typename LeftType, typename RightType>
			bool operator()(const LeftType& leftType, const RightType& rightType) const
			{
				return leftType == rightType;
			}
		};
	}
}
