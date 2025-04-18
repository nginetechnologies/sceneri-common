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

	//! Indicates that the operation should default construct new elements if any have to be created
	enum class DefaultConstructType : uint8
	{
		DefaultConstruct
	};
	//! Indicates that the operation should default construct new elements if any have to be created
	inline static constexpr DefaultConstructType DefaultConstruct = DefaultConstructType::DefaultConstruct;

	//! Indicates that the operation should leave new elements uninitialized if any have to be created
	enum class UninitializedType : uint8
	{
		Uninitialized
	};
	//! Indicates that the operation should leave new elements uninitialized if any have to be created
	inline static constexpr UninitializedType Uninitialized = UninitializedType::Uninitialized;

	//! Indicates that the operation should avoid constructing new elements if any have to be created, and instead fill the data with zeroes
	enum class ZeroedType : uint8
	{
		Zeroed
	};
	//! Indicates that the operation should avoid constructing new elements if any have to be created, and instead fill the data with zeroes
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

	//! Indicates that any attempts to reserve memory / capacity should only allocate the exact memory required, and no more
	enum class ReserveExactType : uint8
	{
		ReserveExact
	};
	//! Indicates that any attempts to reserve memory / capacity should only allocate the exact memory required, and no more
	inline static constexpr ReserveExactType ReserveExact = ReserveExactType::ReserveExact;

	//! Indicates that any attempts to reserve memory / capacity should attempt to grow exponentially
	enum class ReserveExponentialType : uint8
	{
		ReserveExponential
	};
	//! Indicates that any attempts to reserve memory / capacity should attempt to grow exponentially
	inline static constexpr ReserveExponentialType ReserveExponential = ReserveExponentialType::ReserveExponential;

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
