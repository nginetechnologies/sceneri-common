#pragma once

#include <Common/Math/CoreNumericTypes.h>
#include <Common/Memory/Containers/ArrayView.h>
#include <Common/Memory/Containers/String.h>

namespace ngine::CommandLine
{
	struct InitializationParameters
	{
#if PLATFORM_APPLE
		[[nodiscard]] static InitializationParameters& GetGlobalParameters()
		{
			static InitializationParameters parameters;
			return parameters;
		}
#endif

		enum class Type : uint8
		{
			StandardArguments,
			FullCommandLine
		};

		InitializationParameters()
			: m_type(Type::StandardArguments)
			, arguments()
		{
		}

		InitializationParameters(const ArrayView<ConstNativeStringView, uint16> args)
			: m_type(Type::StandardArguments)
			, arguments(args)
		{
		}

#if PLATFORM_WINDOWS
		InitializationParameters(const ConstNativeStringView _fullCommandLine)
			: m_type(Type::FullCommandLine)
			, fullCommandLine(_fullCommandLine)
		{
		}
#endif

		Type m_type;
		union
		{
#if PLATFORM_WINDOWS
			ConstNativeStringView fullCommandLine;
#endif
			ArrayView<ConstNativeStringView, uint16> arguments;
		};
	};
}
