#pragma once

#include <Common/Math/CoreNumericTypes.h>

namespace ngine::IO
{
	enum class SharingFlags : uint8
	{
		AllowAll = 0x40,
		DisallowRead = 0x30,
		DisallowReadWrite = 0x10,
		DisallowWrite = 0x20
	};
}
