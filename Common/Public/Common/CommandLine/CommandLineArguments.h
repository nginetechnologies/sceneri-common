#pragma once

#include "CommandLineArgument.h"

#include <Common/Memory/Containers/Vector.h>

namespace ngine::CommandLine
{
	struct InitializationParameters;
	struct View;

	struct Arguments
	{
		using Container = Vector<Argument, uint16>;
		using ConstView = Container::ConstView;

		Arguments(const InitializationParameters& parameters);

		[[nodiscard]] View GetView() const;

		template<typename Callback>
		void IterateCommands(Callback&& callback) const;
		template<typename Callback>
		void IterateOptions(Callback&& callback) const;

		[[nodiscard]] OptionalIterator<const Argument> FindArgument(const StringViewType name, const Prefix type) const;
		[[nodiscard]] bool HasArgument(const StringViewType name, const Prefix type) const;
		[[nodiscard]] StringViewType GetArgumentValue(const StringViewType name, const Prefix type) const;

		void EmplaceArgument(StringType&& name, StringType&& value, const Prefix prefix);
	protected:
		void Parse(ArrayView<ConstNativeStringView, uint16> commandLineArguments);

#if PLATFORM_WINDOWS
		void Parse(const ConstNativeStringView commandLine);
#endif
	protected:
		Container m_arguments;
	};
}
