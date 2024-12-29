#include "CommandLine/CommandLineArguments.h"
#include "CommandLine/CommandLineInitializationParameters.h"
#include "CommandLine/CommandLineArgumentsView.h"

#include <new>

#include <Common/Math/Select.h>

#if PLATFORM_WINDOWS
#include <Platform/Windows.h>
#include <shellapi.h>
#include <Common/Platform/UndefineWindowsMacros.h>
#endif

namespace ngine::CommandLine
{
	Arguments::Arguments(const InitializationParameters& parameters)
	{
		switch (parameters.m_type)
		{
#if PLATFORM_WINDOWS
			case InitializationParameters::Type::FullCommandLine:
			{
				Parse(parameters.fullCommandLine);
				break;
			}
			case InitializationParameters::Type::StandardArguments:
#else
			default:
#endif
				Parse(parameters.arguments);
				break;
		}
	}

#if PLATFORM_WINDOWS
	void Arguments::Parse(const ConstNativeStringView commandLine)
	{
		int nativeArgumentCount;
		LPWSTR* const pArguments = CommandLineToArgvW(commandLine.GetData(), &nativeArgumentCount);

		FixedCapacityVector<ConstNativeStringView, uint16> commandLineArguments(Memory::Reserve, static_cast<uint16>(nativeArgumentCount));
		for (uint16 i = 0, numArguments = static_cast<uint16>(nativeArgumentCount); i < numArguments; ++i)
		{
			commandLineArguments.EmplaceBack(ConstNativeStringView(pArguments[i], static_cast<uint32>(wcslen(pArguments[i]))));
		}

		struct ScopedFree
		{
			~ScopedFree()
			{
				LocalFree(m_pArguments);
			}

			LPWSTR* const m_pArguments;
		};

		ScopedFree scopedFree{pArguments};

		Parse(commandLineArguments);
	}
#endif

	void Arguments::Parse(ArrayView<ConstNativeStringView, uint16> commandLineArguments)
	{
		m_arguments.Reserve(commandLineArguments.GetSize());

		for (uint16 i = 0, numArguments = commandLineArguments.GetSize(); i < numArguments; ++i)
		{
			const ConstNativeStringView argument = commandLineArguments[i];

			if ((argument[0] == MAKE_NATIVE_LITERAL('-')) | (argument[0] == MAKE_NATIVE_LITERAL('+')))
			{
				const Prefix prefix = Math::Select(argument[0] == MAKE_NATIVE_LITERAL('-'), Prefix::Minus, Prefix::Plus);

				const bool hasValue = (i + 1u) < numArguments && commandLineArguments[i + 1][0] != MAKE_NATIVE_LITERAL('-') &&
				                      commandLineArguments[i + 1][0] != MAKE_NATIVE_LITERAL('+');
				if (hasValue)
				{
					m_arguments.EmplaceBack(
						Argument{StringType(argument.GetSubstring(1, argument.GetSize() - 1)), StringType(commandLineArguments[i + 1u]), prefix}
					);
					i++;
				}
				else
				{
					m_arguments.EmplaceBack(Argument{StringType(argument.GetSubstring(1, argument.GetSize() - 1u)), StringType(), prefix});
				}
			}
			else
			{
				m_arguments.EmplaceBack(Argument{StringType(commandLineArguments[i]), StringType(), Prefix::None});
			}
		}
	}

	View Arguments::GetView() const
	{
		return m_arguments.GetView();
	}

	OptionalIterator<const Argument> Arguments::FindArgument(const StringViewType name, const Prefix type) const
	{
		return GetView().FindArgument(name, type);
	}

	bool Arguments::HasArgument(const StringViewType name, const Prefix type) const
	{
		return GetView().HasArgument(name, type);
	}

	StringViewType Arguments::GetArgumentValue(const StringViewType name, const Prefix type) const
	{
		return GetView().GetArgumentValue(name, type);
	}

	void Arguments::EmplaceArgument(StringType&& name, StringType&& value, const Prefix prefix)
	{
		m_arguments.EmplaceBack(Argument{Forward<StringType>(name), Forward<StringType>(value), prefix});
	}
}
