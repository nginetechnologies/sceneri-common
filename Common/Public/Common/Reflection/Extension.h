#pragma once

#include <Common/Guid.h>

namespace ngine::Reflection
{
	struct ExtensionInterface
	{
		constexpr ExtensionInterface(const Guid guid)
			: m_guid(guid)
		{
		}

		Guid m_guid;
	};

	template<typename... ExtensionTypes>
	struct Extensions : public Tuple<ExtensionTypes...>
	{
		using BaseType = Tuple<ExtensionTypes...>;
		using BaseType::BaseType;
		using BaseType::operator=;
	};
	template<typename... Ts>
	Extensions(const Ts&...) -> Extensions<Ts...>;
	template<typename... Ts>
	Extensions(Ts&&...) -> Extensions<Ts...>;
};
