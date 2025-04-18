#pragma once

#include "../Memory/Containers/StringView.h"
#include "../TypeTraits/WithoutReference.h"
#include "../TypeTraits/WithoutConstOrVolatile.h"

namespace ngine::TypeTraits
{
	namespace Internal
	{
		template<typename T>
		using Remove = WithoutConstOrVolatile<WithoutReference<T>>;

		template<typename... T>
		constexpr ConstStringView N() noexcept
		{
#if COMPILER_CLANG || COMPILER_GCC
			return {__PRETTY_FUNCTION__ + 31, sizeof(__PRETTY_FUNCTION__) - 34};
#elif COMPILER_MSVC
			return {__FUNCSIG__ + 208, (sizeof(__FUNCSIG__) - 208) - 20};
#else
			return {};
#endif
		}

		template<typename... T>
		inline constexpr auto TypeName = N<T...>();
	}

	template<typename T>
	[[nodiscard]] constexpr ConstStringView GetTypeName() noexcept
	{
		using W = Internal::Remove<T>;
		constexpr ConstStringView name = Internal::TypeName<W>;
		static_assert(name.GetSize() > 0, "Could not obtain type name.");

		return name;
	}
};
