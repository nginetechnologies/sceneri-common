#pragma once

#include <Common/Asset/AssetFormat.h>

namespace ngine
{
	inline static constexpr Asset::Format PluginAssetFormat = {
		"{926268B2-2D45-4C52-87C9-D76815217359}"_asset, MAKE_PATH(".nplugin"), {}, Asset::Format::Flags{}
	};
}
