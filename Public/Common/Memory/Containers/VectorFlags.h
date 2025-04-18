#pragma once

namespace ngine::Memory
{
	struct VectorFlags
	{
		enum VectorFlagsType : uint8
		{
			None = 0,
			AllowReallocate = 1 << 0,
			AllowResize = 1 << 1
		};
	};
}
