#pragma once

#include "Ratio.h"
#include "Vector3.h"

#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename T>
	struct TRIVIAL_ABI TScale3 : public TVector3<TRatio<T>>
	{
		inline static constexpr Guid TypeGuid = "f4aec1bf-99d3-41f5-ace5-2d00b7b44a50"_guid;

		using ContainedType = TRatio<T>;
		using BaseType = TVector3<TRatio<T>>;

		using BaseType::BaseType;

		FORCE_INLINE constexpr TScale3(const BaseType value) noexcept
			: BaseType(value)
		{
		}
		FORCE_INLINE constexpr TScale3(const TVector3<T> value) noexcept
			: BaseType{value.x, value.y, value.z}
		{
		}

		FORCE_INLINE constexpr TScale3(const ContainedType _x, const ContainedType _y, const ContainedType _z) noexcept
			: BaseType(_x, _y, _z)
		{
		}
	};

	using Scale3f = TScale3<float>;
}
