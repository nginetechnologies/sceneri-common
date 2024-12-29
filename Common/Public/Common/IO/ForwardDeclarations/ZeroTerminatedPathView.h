#pragma once

#include <Common/Memory/Containers/ForwardDeclarations/ZeroTerminatedStringView.h>
#include <Common/Memory/NativeCharType.h>

namespace ngine::IO
{
	template<typename CharType>
	using TZeroTerminatedPathView = TZeroTerminatedStringView<CharType, uint16>;

	using ConstZeroTerminatedPathView = TZeroTerminatedPathView<const NativeCharType>;
	using ZeroTerminatedPathView = TZeroTerminatedPathView<NativeCharType>;
}
