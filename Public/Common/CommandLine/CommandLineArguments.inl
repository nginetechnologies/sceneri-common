#pragma once

#include "CommandLineArguments.h"
#include "CommandLineArgumentsView.h"

namespace ngine::CommandLine
{
	template<typename Callback>
	inline void Arguments::IterateCommands(Callback&& callback) const
	{
		Arguments::Container::ConstView arguments = GetView();
		for (auto it = arguments.begin(); it != arguments.end();)
		{
			switch (it->prefix)
			{
				case Prefix::Plus:
				{
					const ArgumentView command = *it;
					++it;
					Arguments::Container::ConstView commandArguments{it, it};
					for (; it != arguments.end() && it->prefix == Prefix::Minus; ++it)
					{
						commandArguments = {commandArguments.begin(), it + 1};
					}
					callback(command, View(commandArguments));
				}
				break;
				default:
					++it;
					break;
			}
		}
	}

	template<typename Callback>
	inline void Arguments::IterateOptions(Callback&& callback) const
	{
		Arguments::Container::ConstView arguments = GetView();
		for (auto it = arguments.begin(); it != arguments.end();)
		{
			switch (it->prefix)
			{
				case Prefix::Minus:
				{
					const ArgumentView command = *it;
					++it;
					callback(command);
				}
				break;
				default:
					++it;
					break;
			}
		}
	}
}
