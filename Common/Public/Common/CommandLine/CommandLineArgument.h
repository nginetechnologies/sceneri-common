#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Memory/Containers/String.h>

namespace ngine::CommandLine
{
	enum class Prefix : uint8
	{
		None = 0,
		Minus,
		Plus
	};

	using StringType = TString<NativeCharType, Memory::DynamicAllocator<NativeCharType, uint16>, Memory::VectorFlags::None>;
	using StringViewType = TStringView<const NativeCharType, uint16>;

	struct Argument
	{
		StringType key;
		StringType value;

		Prefix prefix;
	};

	struct ArgumentView
	{
		ArgumentView(const Argument& argument)
			: key(argument.key.GetView())
			, value(argument.value.GetView())
			, prefix(argument.prefix)
		{
		}

		StringViewType key;
		StringViewType value;

		Prefix prefix;
	};
}
