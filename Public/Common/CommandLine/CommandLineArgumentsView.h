#pragma once

#include "CommandLineArgument.h"

#include <Common/Platform/TrivialABI.h>

namespace ngine::CommandLine
{
	struct TRIVIAL_ABI View : public ArrayView<const Argument, uint16>
	{
		using BaseType = ArrayView<const Argument, uint16>;

		using BaseType::BaseType;
		using BaseType::operator=;
		View(const BaseType& view)
			: BaseType(view)
		{
		}

		[[nodiscard]] OptionalIterator<const Argument> FindArgument(const StringViewType name, const Prefix type) const
		{
			return FindIf(
				[name, type](const Argument& __restrict argument) -> bool
				{
					return argument.prefix == type && argument.key == name;
				}
			);
		}

		[[nodiscard]] bool HasArgument(const StringViewType name, const Prefix type) const
		{
			return FindArgument(name, type).IsValid();
		}

		[[nodiscard]] StringViewType GetArgumentValue(const StringViewType name, const Prefix type) const
		{
			if (const OptionalIterator<const Argument> pArgument = FindArgument(name, type))
			{
				return pArgument->value;
			}
			else
			{
				return {};
			}
		}
	};
}
