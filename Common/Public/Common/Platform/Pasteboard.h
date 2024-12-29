#pragma once

#include <Common/Memory/Containers/ForwardDeclarations/StringView.h>
#include <Common/Memory/Containers/ForwardDeclarations/String.h>
#include <Common/Function/ForwardDeclarations/Function.h>

namespace ngine::Platform
{
	struct Pasteboard
	{
		static bool PasteText(const ConstUnicodeStringView text);

		using GetTextCallback = Function<void(UnicodeString&&), 24>;
		static bool GetText(GetTextCallback&& callback);
	};
}
