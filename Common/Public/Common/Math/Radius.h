#pragma once

#include "Length.h"
#include <Common/Math/ForwardDeclarations/Radius.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Math
{
	template<typename Type>
	struct TRIVIAL_ABI Radius : public Length<Type>
	{
		inline static constexpr Guid TypeGuid = "{5906FF41-6FE2-4CED-957B-570D8E5AEBD1}"_guid;

		using BaseType = Length<Type>;
		using BaseType::BaseType;
		using BaseType::operator=;
		constexpr Radius(const BaseType& length)
			: BaseType(length)
		{
		}
		constexpr Radius& operator=(const BaseType& length)
		{
			return BaseType::operator=(length);
		}

		FORCE_INLINE constexpr Radius(ZeroType zero) noexcept
			: BaseType(zero)
		{
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Radius FromMeters(const Type value) noexcept
		{
			return BaseType::FromMeters(value);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Radius FromMillimeters(const Type value) noexcept
		{
			return BaseType::FromMillimeters(value);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Radius FromCentimeters(const Type value) noexcept
		{
			return BaseType::FromCentimeters(value);
		}

		[[nodiscard]] FORCE_INLINE PURE_STATICS static constexpr Radius FromInches(const Type value) noexcept
		{
			return BaseType::FromInches(value);
		}
	};
}
