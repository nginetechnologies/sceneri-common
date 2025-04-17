#pragma once

#include <Common/Guid.h>
#include <Common/Platform/TrivialABI.h>

namespace ngine::Asset
{
	struct TRIVIAL_ABI Guid : public ngine::Guid
	{
		using BaseType = ngine::Guid;

		using BaseType::BaseType;
		constexpr Guid(const BaseType& other)
			: BaseType(other)
		{
		}

		static Guid Generate()
		{
			return Guid(BaseType::Generate());
		}

		static Guid TryParse(const ConstStringView input)
		{
			return Guid(BaseType::TryParse(input));
		}
		template<typename NativeCharType_ = NativeCharType, typename = EnableIf<!TypeTraits::IsSame<char, NativeCharType_>>>
		static Guid TryParse(const ConstNativeStringView input)
		{
			return Guid(BaseType::TryParse(input));
		}
	};

	namespace Literals
	{
		constexpr Guid operator""_asset(const char* szInput, size n)
		{
			return Guid(ConstStringView(szInput, static_cast<uint32>(n)));
		}
	}

	using namespace Literals;
}

namespace ngine
{
	using namespace Asset::Literals;
}
