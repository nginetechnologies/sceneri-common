#pragma once

#include <Common/IO/PathView.h>
#include <Common/EnumFlags.h>

namespace ngine::Asset
{
	struct Format
	{
		enum class Flags : uint8
		{
			None,
			IsCollection = 1 << 0
		};

		ngine::Guid assetTypeGuid;
		IO::PathView metadataFileExtension;
		IO::PathView binaryFileExtension;
		EnumFlags<Flags> flags;
	};

	inline static constexpr Format AssetFormat = {"{F2B5F555-BFF3-476C-BDE1-1B6BA24E70BB}"_guid, MAKE_PATH(".nasset"), {}, Format::Flags{}};
}
